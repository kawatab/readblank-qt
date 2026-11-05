#include "SpeechController.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>

SpeechController::SpeechController(QObject *parent) : QObject(parent)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);

    m_network = new QNetworkAccessManager(this);

    m_player->setAudioOutput(m_audioOutput);

    connect(m_player, &QMediaPlayer::playbackStateChanged, this,
            [this](QMediaPlayer::PlaybackState) { emit speakingChanged(); });

    connect(m_player, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &message) {
                m_errorString = message;
                emit errorChanged();
            });

    QSettings settings;
    m_lengthScale = qBound(0.5, settings.value("speechLengthScale", 1.0).toDouble(), 2.0);
    m_audioOutput->setVolume(qBound(0.0, settings.value("speechVolume", 1.0).toDouble(), 1.0));
}

bool SpeechController::speaking() const
{
    return m_player->playbackState() == QMediaPlayer::PlayingState;
}

double SpeechController::lengthScale() const
{
    return m_lengthScale;
}

void SpeechController::setLengthScale(double value)
{
    value = qBound(0.5, value, 2.0);

    if (qFuzzyCompare(m_lengthScale, value))
        return;

    m_lengthScale = value;

    QSettings settings;
    settings.setValue("speechLengthScale", m_lengthScale);

    emit lengthScaleChanged();
}

double SpeechController::volume() const
{
    return m_audioOutput->volume();
}

void SpeechController::setVolume(double value)
{
    value = qBound(0.0, value, 1.0);

    if (qFuzzyCompare(m_audioOutput->volume(), static_cast<float>(value)))
        return;

    m_audioOutput->setVolume(value);

    QSettings settings;
    settings.setValue("speechVolume", value);

    emit volumeChanged();
}

void SpeechController::speak(const QString &text, const QString &language)
{
    stop();

    m_errorString.clear();
    emit errorChanged();

    QJsonObject json;
    json["text"] = text;
    json["voice"] = voiceForLanguage(language);
    json["length_scale"] = m_lengthScale;

    QNetworkRequest request(QUrl("http://127.0.0.1:5000/synthesize"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_reply = m_network->post(request, QJsonDocument(json).toJson(QJsonDocument::Compact));

    connect(m_reply, &QNetworkReply::finished, this, [this, reply = m_reply]() {
        if (m_reply != reply) {
            reply->deleteLater();
            return;
        }

        m_reply = nullptr;

        if (reply->error() != QNetworkReply::NoError) {
            m_errorString = reply->errorString();
            emit errorChanged();
            reply->deleteLater();
            return;
        }

        const QByteArray audioData = reply->readAll();

        reply->deleteLater();

        playAudio(audioData);
    });
}

QString SpeechController::voiceForLanguage(const QString &language) const
{
    const QString lang = language.toLower().section('-', 0, 0);

    if (lang == "hu")
        return "hu_HU-anna-medium";

    if (lang == "fr")
        return "fr_FR-siwis-medium";

    return "en_US-lessac-medium";
}

QString SpeechController::errorString() const
{
    return m_errorString;
}

void SpeechController::playAudio(const QByteArray &audioData)
{
    m_player->stop();
    m_player->setSourceDevice(nullptr);

    if (m_audioBuffer) {
        m_audioBuffer->deleteLater();
        m_audioBuffer = nullptr;
    }

    auto *buffer = new QBuffer(this);
    buffer->setData(audioData);

    if (!buffer->open(QIODevice::ReadOnly)) {
        m_errorString = "Failed to open audio buffer.";
        emit errorChanged();
        buffer->deleteLater();
        return;
    }

    m_audioBuffer = buffer;
    m_player->setSourceDevice(buffer);
    m_player->play();
}

void SpeechController::stop()
{
    if (m_reply) {
        QNetworkReply *reply = m_reply;
        m_reply = nullptr;
        reply->abort();
        reply->deleteLater();
    }

    m_player->stop();
}
