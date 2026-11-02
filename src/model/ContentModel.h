// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>

class ContentModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QDateTime lastAccessed READ lastAccessed NOTIFY lastAccessedChanged)
    Q_PROPERTY(QString content READ content NOTIFY contentChanged)
    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)

public:
    explicit ContentModel(QObject *parent = nullptr);

    QString url() const;
    QString title() const;
    QString language() const;
    QDateTime lastAccessed() const;
    QString content() const;

    bool loaded() const;

    void setUrl(const QString &url);
    void setTitle(const QString &title);
    void setLanguage(const QString &language);
    void setLastAccessed(const QDateTime &dateTime);

    void setMetadata(const QString &title, const QString &language);

    void setContent(const QString &content);
    void clearCache();

signals:
    void urlChanged();
    void titleChanged();
    void languageChanged();
    void lastAccessedChanged();
    void contentChanged();
    void loadedChanged();

private:
    QString m_url;
    QString m_title;
    QString m_language;
    QDateTime m_lastAccessed;
    QString m_content;

    bool m_loaded = false;
};
