// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QUrl>

#include "../model/ContentModel.h"

class ContentController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString content READ content NOTIFY contentChanged)
    Q_PROPERTY(QString language READ language NOTIFY languageChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QObject *currentModel READ currentModel NOTIFY currentModelChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

    Q_PROPERTY(QString currentParagraph READ currentParagraph NOTIFY paragraphStateChanged)
    Q_PROPERTY(int paragraphCount READ paragraphCount NOTIFY paragraphStateChanged)
    Q_PROPERTY(bool canGoPrevious READ canGoPrevious NOTIFY paragraphStateChanged)
    Q_PROPERTY(bool canGoNext READ canGoNext NOTIFY paragraphStateChanged)


public:
    explicit ContentController(QObject *parent = nullptr);

    QString title() const;
    QString content() const;
    QString language() const;
    bool loading() const;
    QObject *currentModel() const;
    QString errorString() const;

    Q_INVOKABLE void loadFavorite(QObject *object);
    Q_INVOKABLE void cancelLoading();

    QString currentParagraph() const;
    int paragraphCount() const;
    bool canGoPrevious() const;
    bool canGoNext() const;

    Q_INVOKABLE void previousParagraph();
    Q_INVOKABLE void nextParagraph();

signals:
    void titleChanged();
    void paragraphStateChanged();
    void contentChanged();
    void languageChanged();
    void loadingChanged();
    void currentModelChanged();
    void errorChanged();

private:
    void cancelCurrentRequest();

    QString m_content;
    int m_paragraphIndex = 0;

    QPointer<ContentModel> m_currentModel;
    QMetaObject::Connection m_titleConnection;
    QMetaObject::Connection m_paragraphsConnection;
    QMetaObject::Connection m_languageConnection; 
    bool m_loading = false;
    QString m_errorString;
    QNetworkAccessManager m_networkManager;
    QNetworkReply *m_currentReply = nullptr;
};
