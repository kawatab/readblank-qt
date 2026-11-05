// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#include "ContentController.h"

#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <algorithm>
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

ContentController::ContentController(QObject *parent) : QObject(parent) { }

QString ContentController::title() const
{
    return m_currentModel ? m_currentModel->title() : QString();
}

QString ContentController::content() const
{
    return m_content;
}

QString ContentController::language() const
{
    return m_currentModel ? m_currentModel->language() : QString();
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
    disconnect(m_paragraphsConnection);
    disconnect(m_languageConnection);

    // Set the current model.
    m_currentModel = model;
    m_paragraphIndex = 0;

    updateWordSelection();

    emit currentModelChanged();
    emit languageChanged();
    emit paragraphStateChanged();
    emit titleChanged();

    // Observe title changes of the current model.
    if (model) {
        m_titleConnection =
                connect(model, &ContentModel::titleChanged, this, &ContentController::titleChanged);

        m_paragraphsConnection = connect(model, &ContentModel::contentChanged, this, [this]() {
            updateWordSelection();
            emit paragraphStateChanged();
        });

        m_languageConnection = connect(model, &ContentModel::languageChanged, this,
                                       &ContentController::languageChanged);

        model->setLastAccessed(QDateTime::currentDateTime());
    }

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
            emit contentChanged();
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

int ContentController::currentParagraphIndex() const
{
    return m_paragraphIndex;
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
    if (canGoPrevious())
        setCurrentParagraph(m_paragraphIndex - 1);
}

void ContentController::nextParagraph()
{
    if (canGoNext())
        setCurrentParagraph(m_paragraphIndex + 1);
}

void ContentController::firstParagraph()
{
    setCurrentParagraph(0);
}

void ContentController::lastParagraph()
{
    setCurrentParagraph(paragraphCount() - 1);
}

void ContentController::setCurrentParagraph(int index)
{
    if (index < 0 || index >= paragraphCount()
        || index == m_paragraphIndex)
        return;

    m_paragraphIndex = index;

    updateWordSelection();
    emit currentParagraphChanged();
    emit paragraphStateChanged();
}

void ContentController::updateWordSelection()
{
    const QString paragraph = currentParagraph();

    m_words.clear();
    m_selectableWordIndexes.clear();
    m_usedButtonIndexes.clear();
    m_revealedWordIndexes.clear();
    m_selectedWordIndex = -1;

    // Extract words.
    int i = 0;
    int wordIndex = 0;

    while (i < paragraph.size()) {
        if (!paragraph.at(i).isLetter()) {
            ++i;
            continue;
        }

        const int start = i;
        ++i;

        while (i < paragraph.size()) {
            if (paragraph.at(i).isLetter()) {
                ++i;
            } else if (paragraph.at(i) == QLatin1Char('\'') && i + 1 < paragraph.size()
                       && paragraph.at(i + 1).isLetter()) {
                ++i;
            } else {
                break;
            }
        }

        m_words.append({ start, i - start, wordIndex });
        ++wordIndex;
    }

    const int wordCount = m_words.size();

    if (wordCount == 0) {
        updateParagraphHtml();
        return;
    }

    // Select 30% of the words, rounding up.
    const int selectableCount = (wordCount * 3 + 9) / 10;

    QVector<int> candidates;

    for (int n = 0; n < wordCount; ++n)
        candidates.append(n);

    // Shuffle the candidates.
    for (int n = candidates.size() - 1; n > 0; --n) {
        const int j = int(QRandomGenerator::global()->bounded(quint32(n + 1)));
        candidates.swapItemsAt(n, j);
    }

    candidates.resize(selectableCount);

    std::sort(candidates.begin(), candidates.end());

    m_selectableWordIndexes = candidates;

    m_selectedWordIndex = m_selectableWordIndexes.first();

    updateParagraphHtml();
}

void ContentController::updateParagraphHtml()
{
    QString html;
    int position = 0;

    for (int i = 0; i < m_words.size(); ++i) {
        const auto &word = m_words.at(i);

        html += currentParagraph().mid(position, word.start - position).toHtmlEscaped();

        const QString text = currentParagraph().mid(word.start, word.length).toHtmlEscaped();

        if (m_selectableWordIndexes.contains(i)) {
            const QString background = (i == m_selectedWordIndex) ? "#f0a040" : "#d0d0d0";

            if (m_revealedWordIndexes.contains(i)) {

                html += QStringLiteral(
                                "<span style=\"background-color:%1;color:#202020;\">%2</span>")
                                .arg(background, text);
            } else {
                html += QStringLiteral("<span "
                                       "style=\"background-color:%1;color:transparent;\">%2</span>")
                                .arg(background, text);
            }
        } else {
            html += text;
        }

        position = word.start + word.length;
    }

    html += currentParagraph().mid(position).toHtmlEscaped();

    m_currentParagraphHtml = html;
}

QString ContentController::currentParagraphHtml() const
{
    return m_currentParagraphHtml;
}

bool ContentController::canSelectPreviousWord() const
{
    const int position = m_selectableWordIndexes.indexOf(m_selectedWordIndex);

    return position > 0;
}

bool ContentController::canSelectNextWord() const
{
    const int position = m_selectableWordIndexes.indexOf(m_selectedWordIndex);

    return position >= 0 && position + 1 < m_selectableWordIndexes.size();
}

void ContentController::selectWord(int wordIndex)
{
    if (!m_selectableWordIndexes.contains(wordIndex))
        return;

    if (m_selectedWordIndex == wordIndex)
        return;

    m_selectedWordIndex = wordIndex;

    updateParagraphHtml();
    emit paragraphStateChanged();
}

void ContentController::previousWord()
{
    if (!canSelectPreviousWord())
        return;

    const int position = m_selectableWordIndexes.indexOf(m_selectedWordIndex);

    m_selectedWordIndex = m_selectableWordIndexes.at(position - 1);

    updateParagraphHtml();
    emit paragraphStateChanged();
}

void ContentController::nextWord()
{
    if (!canSelectNextWord())
        return;

    const int position = m_selectableWordIndexes.indexOf(m_selectedWordIndex);

    m_selectedWordIndex = m_selectableWordIndexes.at(position + 1);

    updateParagraphHtml();
    emit paragraphStateChanged();
}

QVariantList ContentController::selectableWords() const
{
    QVariantList result;

    QVector<int> indexes = m_selectableWordIndexes;

    std::sort(indexes.begin(), indexes.end(), [this](int a, int b) {
        const auto &wordA = m_words.at(a);
        const auto &wordB = m_words.at(b);

        const QString textA = currentParagraph().mid(wordA.start, wordA.length);
        const QString textB = currentParagraph().mid(wordB.start, wordB.length);

        return QString::compare(textA, textB, Qt::CaseInsensitive) < 0;
    });

    for (int index : indexes) {
        const auto &word = m_words.at(index);

        QVariantMap item;
        item.insert("index", index);
        item.insert("text", currentParagraph().mid(word.start, word.length).toLower());
        item.insert("revealed", m_usedButtonIndexes.contains(index));

        result.append(item);
    }

    return result;
}

int ContentController::selectedWordIndex() const
{
    return m_selectedWordIndex;
}

void ContentController::revealWord(int wordIndex)
{
    if (!m_selectableWordIndexes.contains(wordIndex))
        return;

    if (m_usedButtonIndexes.contains(wordIndex))
        return;

    if (m_selectedWordIndex < 0)
        return;

    const auto &buttonWord = m_words.at(wordIndex);
    const auto &selectedWord = m_words.at(m_selectedWordIndex);

    const QString buttonText = currentParagraph().mid(buttonWord.start, buttonWord.length);

    const QString selectedText = currentParagraph().mid(selectedWord.start, selectedWord.length);

    if (QString::compare(buttonText, selectedText, Qt::CaseInsensitive) != 0)
        return;

    m_usedButtonIndexes.insert(wordIndex);
    m_revealedWordIndexes.insert(m_selectedWordIndex);

    emit wordRevealed(selectedText, language());

    if (canSelectNextWord())
        nextWord();
    else {
        updateParagraphHtml();
        emit paragraphStateChanged();
    }
}
