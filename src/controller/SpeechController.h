#pragma once

#include <QAudioOutput>
#include <QBuffer>
#include <QMediaPlayer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>

class SpeechController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool speaking READ speaking NOTIFY speakingChanged)
    Q_PROPERTY(double lengthScale READ lengthScale WRITE setLengthScale NOTIFY lengthScaleChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

public:
    explicit SpeechController(QObject *parent = nullptr);

    bool speaking() const;
    double lengthScale() const;
    void setLengthScale(double value);
    QString errorString() const;

    Q_INVOKABLE void speak(const QString &text, const QString &language);
    Q_INVOKABLE QString voiceForLanguage(const QString &language) const;
    Q_INVOKABLE void stop();

signals:
    void speakingChanged();
    void lengthScaleChanged();
    void errorChanged();

private:
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    QPointer<QBuffer> m_audioBuffer;
    double m_lengthScale = 1.0;
    QString m_errorString;

    QNetworkAccessManager *m_network = nullptr;
    QNetworkReply *m_reply = nullptr;

    void playAudio(const QByteArray &audioData);
};
