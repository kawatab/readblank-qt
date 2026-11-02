// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QUrl>

#include "../model/ContentModel.h"

class ContentController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString content READ content NOTIFY contentChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QObject *currentModel READ currentModel NOTIFY currentModelChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

public:
    explicit ContentController(QObject *parent = nullptr);

    QString content() const;
    bool loading() const;
    QObject *currentModel() const;
    QString errorString() const;

    Q_INVOKABLE void loadFavorite(QObject *object);

signals:
    void contentChanged();
    void loadingChanged();
    void currentModelChanged();
    void errorChanged();

private:
    void cancelCurrentRequest();

    QNetworkAccessManager m_networkManager;
    QNetworkReply *m_currentReply = nullptr;

    QPointer<ContentModel> m_currentModel;

    QString m_content;
    QString m_errorString;
    bool m_loading = false;
};
