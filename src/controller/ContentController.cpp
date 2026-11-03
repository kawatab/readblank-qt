// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#include "ContentController.h"

#include <QNetworkRequest>
#include <QRegularExpression>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>

namespace {

struct ParsedContent
{
    QString title;
    QString language;
    QStringList paragraphs;
};

QString nodeText(xmlNode *node)
{
    xmlChar *text = xmlNodeGetContent(node);

    if (!text)
        return {};

    const QString result = QString::fromUtf8(reinterpret_cast<const char *>(text)).simplified();

    xmlFree(text);

    return result;
}

void findElements(xmlNode *node, ParsedContent &result, QStringList &paragraphs)
{
    for (xmlNode *current = node; current; current = current->next) {

        if (current->type == XML_ELEMENT_NODE) {
            if (xmlStrcasecmp(current->name, BAD_CAST "title") == 0) {

                if (result.title.isEmpty())
                    result.title = nodeText(current);
            }

            else if (xmlStrcasecmp(current->name, BAD_CAST "p") == 0) {

                const QString paragraph = nodeText(current);

                if (!paragraph.isEmpty())
                    paragraphs.append(paragraph);
            }
        }

        findElements(current->children, result, paragraphs);
    }
}

ParsedContent parseHtml(const QByteArray &html, const QByteArray &encoding = {})
{
    ParsedContent result;

    const char *encodingName = encoding.isEmpty() ? nullptr : encoding.constData();

    htmlDocPtr doc = htmlReadMemory(html.constData(), html.size(), nullptr, encodingName,
                                    HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING
                                            | HTML_PARSE_NONET);

    if (!doc)
        return result;

    xmlNode *root = xmlDocGetRootElement(doc);

    if (root) {
        xmlChar *language = xmlGetProp(root, BAD_CAST "lang");

        if (language) {
            result.language = QString::fromUtf8(reinterpret_cast<const char *>(language));

            xmlFree(language);
        }
    }

    QStringList paragraphs;

    findElements(root, result, paragraphs);

    result.paragraphs = paragraphs;

    xmlFreeDoc(doc);

    return result;
}

QByteArray getEncoding(const QString &contentType)
{
    static const QRegularExpression expression(R"(charset\s*=\s*["']?([^;"'\s]+))",
                                               QRegularExpression::CaseInsensitiveOption);

    const auto match = expression.match(contentType);

    if (!match.hasMatch())
        return {};

    return match.captured(1).toLatin1();
}

} // namespace

ContentController::ContentController(QObject *parent) : QObject(parent)
{
    m_speech = new QTextToSpeech(this);

    connect(m_speech, &QTextToSpeech::stateChanged, this,
            [this](QTextToSpeech::State) { emit speechStateChanged(); });
}

QString ContentController::title() const
{
    return m_currentModel ? m_currentModel->title() : QString();
}

QString ContentController::content() const
{
    return m_content;
}

bool ContentController::loading() const
{
    return m_loading;
}

QObject *ContentController::currentModel() const
{
    return m_currentModel.data();
}

QString ContentController::errorString() const
{
    return m_errorString;
}

void ContentController::cancelCurrentRequest()
{
    if (!m_currentReply)
        return;

    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;

    reply->abort();

    if (m_loading) {
        m_loading = false;
        emit loadingChanged();
    }
}

void ContentController::loadFavorite(QObject *object)
{
    ContentModel *model = qobject_cast<ContentModel *>(object);

    cancelCurrentRequest();

    // Disconnect the previous model.
    disconnect(m_titleConnection);

    // Set the current model.
    m_currentModel = model;
    m_paragraphIndex = 0;
    updateSpeechVoice();

    emit paragraphStateChanged();
    emit titleChanged();

    // Observe title changes of the current model.
    if (model) {
        m_titleConnection =
                connect(model, &ContentModel::titleChanged, this, &ContentController::titleChanged);
        m_paragraphsConnection = connect(model, &ContentModel::contentChanged, this,
                                         &ContentController::paragraphStateChanged);
    }

    model->setLastAccessed(QDateTime::currentDateTime());

    if (!model) {
        if (!m_content.isEmpty()) {
            m_content.clear();
            emit contentChanged();
        }

        m_errorString.clear();
        emit errorChanged();
        return;
    }

    // Use the cached content if available.
    if (model->loaded()) {
        if (m_content != model->content()) {
            m_content = model->content();
            emit titleChanged();
        }

        m_errorString.clear();
        emit errorChanged();
        return;
    }

    const QUrl url(model->url());

    if (!url.isValid() || url.isEmpty()) {
        m_errorString = QStringLiteral("Invalid URL");
        emit errorChanged();
        return;
    }

    if (!m_content.isEmpty()) {
        m_content.clear();
        emit contentChanged();
    }

    m_errorString.clear();
    emit errorChanged();

    m_loading = true;
    emit loadingChanged();

    QNetworkRequest request(url);

    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_networkManager.get(request);

    m_currentReply = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        // Ignore responses from obsolete requests.
        if (reply != m_currentReply)
            return;

        m_currentReply = nullptr;

        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (reply->error() == QNetworkReply::NoError && (statusCode == 0 || statusCode < 400)) {

            const QByteArray html = reply->readAll();

            const QString contentType =
                    reply->header(QNetworkRequest::ContentTypeHeader).toString();

            const ParsedContent parsed = parseHtml(html, getEncoding(contentType));

            if (m_currentModel) {
                m_currentModel->setMetadata(parsed.title, parsed.language);
                updateSpeechVoice();

                m_currentModel->setParagraphs(parsed.paragraphs);
            }

            m_content = parsed.paragraphs.join('\n');
            emit contentChanged();

        } else {
            m_errorString = reply->errorString();

            if (statusCode >= 400) {
                m_errorString += QStringLiteral(" (HTTP %1)").arg(statusCode);
            }

            emit errorChanged();
        }

        m_loading = false;
        emit loadingChanged();
    });
}

void ContentController::cancelLoading()
{
    cancelCurrentRequest();
}

QString ContentController::currentParagraph() const
{
    if (!m_currentModel)
        return {};

    const QStringList paragraphs = m_currentModel->paragraphs();

    if (m_paragraphIndex < 0 || m_paragraphIndex >= paragraphs.size()) {
        return {};
    }

    return paragraphs.at(m_paragraphIndex);
}

int ContentController::paragraphCount() const
{
    return m_currentModel ? m_currentModel->paragraphs().size() : 0;
}

bool ContentController::canGoPrevious() const
{
    return m_currentModel && m_paragraphIndex > 0;
}

bool ContentController::canGoNext() const
{
    return m_currentModel && m_paragraphIndex + 1 < paragraphCount();
}

void ContentController::previousParagraph()
{
    if (!canGoPrevious())
        return;

    m_speech->stop();

    --m_paragraphIndex;
    emit paragraphStateChanged();
}

void ContentController::nextParagraph()
{
    if (!canGoNext())
        return;

    m_speech->stop();

    ++m_paragraphIndex;
    emit paragraphStateChanged();
}

bool ContentController::speechActive() const
{
    return m_speech && m_speech->state() != QTextToSpeech::Ready;
}

QString ContentController::speechEngineName() const
{
    return QStringLiteral("eSpeak NG (via Speech Dispatcher)");
}

QString ContentController::speechVoiceName() const
{
    if (!m_speech)
        return {};

    return m_speech->voice().name();
}

QString ContentController::speechLicense() const
{
    return QStringLiteral("GPL-3.0-or-later");
}

QString ContentController::speechProjectUrl() const
{
    return QStringLiteral("https://github.com/espeak-ng/espeak-ng");
}

bool ContentController::speechVoiceFallback() const
{
    return m_speechVoiceFallback;
}

void ContentController::speakCurrentParagraph()
{
    if (!m_currentModel || !m_speech)
        return;

    const QString text = currentParagraph();

    if (text.isEmpty())
        return;

    m_speech->stop();
    m_speech->say(text);
}

void ContentController::stopSpeaking()
{
    if (m_speech)
        m_speech->stop();
}

void ContentController::updateSpeechVoice()
{
    if (!m_speech || !m_currentModel)
        return;

    m_speechVoiceFallback = false;
    const QString language = m_currentModel->language().trimmed();

    // Default to English when the language is unavailable.
    const QString localeName = language.isEmpty() ? QStringLiteral("en") : language;

    QString normalizedLocaleName = localeName;
    normalizedLocaleName.replace('-', '_');

    const QLocale requestedLocale(normalizedLocaleName);

    if (requestedLocale.language() == QLocale::C) {
        qWarning() << "Invalid language tag:" << language;
        return;
    }

    const QList<QLocale> availableLocales = m_speech->availableLocales();

    QLocale selectedLocale;
    bool localeFound = false;

    // Prefer a voice with the exact language and territory.
    for (const QLocale &locale : availableLocales) {
        if (locale.language() != requestedLocale.language())
            continue;

        if (requestedLocale.territory() != QLocale::AnyTerritory
            && locale.territory() == requestedLocale.territory()) {
            selectedLocale = locale;
            localeFound = true;
            break;
        }

        // Keep the first voice for the same language as a fallback.
        if (!localeFound) {
            selectedLocale = locale;
            localeFound = true;
        }
    }

    if (!localeFound && requestedLocale.language() != QLocale::English) {
        // Fall back to English.
        for (const QLocale &locale : availableLocales) {
            if (locale.language() == QLocale::English) {
                selectedLocale = locale;
                localeFound = true;
                m_speechVoiceFallback = true;
                break;
            }
        }
    }

    if (!localeFound) {
        qWarning() << "No suitable speech voice available for:" << language;
        return;
    }

    m_speech->setLocale(selectedLocale);

    if (m_speech->availableVoices().isEmpty()) {
        qWarning() << "No speech voice available for locale:" << selectedLocale;
        return;
    }

    emit speechInfoChanged();
}
