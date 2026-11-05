// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QUrl>
#include <QVariantList>
#include <QVector>

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
    Q_PROPERTY(int currentParagraphIndex READ currentParagraphIndex NOTIFY paragraphStateChanged)
    Q_PROPERTY(bool canGoPrevious READ canGoPrevious NOTIFY paragraphStateChanged)
    Q_PROPERTY(bool canGoNext READ canGoNext NOTIFY paragraphStateChanged)

    Q_PROPERTY(QString currentParagraphHtml READ currentParagraphHtml NOTIFY paragraphStateChanged)
    Q_PROPERTY(bool canSelectPreviousWord READ canSelectPreviousWord NOTIFY paragraphStateChanged)
    Q_PROPERTY(bool canSelectNextWord READ canSelectNextWord NOTIFY paragraphStateChanged)

    Q_PROPERTY(QVariantList selectableWords READ selectableWords NOTIFY paragraphStateChanged)
    Q_PROPERTY(int selectedWordIndex READ selectedWordIndex NOTIFY paragraphStateChanged)
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
    int currentParagraphIndex() const;
    bool canGoPrevious() const;
    bool canGoNext() const;

    Q_INVOKABLE void previousParagraph();
    Q_INVOKABLE void nextParagraph();
    Q_INVOKABLE void firstParagraph();
    Q_INVOKABLE void lastParagraph();
    Q_INVOKABLE void setCurrentParagraph(int index);

    QString currentParagraphHtml() const;

    bool canSelectPreviousWord() const;
    bool canSelectNextWord() const;
    QVariantList selectableWords() const;
    int selectedWordIndex() const;

    Q_INVOKABLE void selectWord(int wordIndex);
    Q_INVOKABLE void previousWord();
    Q_INVOKABLE void nextWord();
    Q_INVOKABLE void revealWord(int wordIndex);

signals:
    void titleChanged();
    void paragraphStateChanged();
    void currentParagraphChanged();
    void contentChanged();
    void languageChanged();
    void loadingChanged();
    void currentModelChanged();
    void wordRevealed(const QString &word, const QString &language);
    void errorChanged();

private:
    void cancelCurrentRequest();
    void updateWordSelection();
    void updateParagraphHtml();

    QString m_content;
    int m_paragraphIndex = 0;
    QVector<int> m_selectableWordIndexes;
    int m_selectedWordIndex = -1;
    QString m_currentParagraphHtml;

    struct WordRange
    {
        int start;
        int length;
        int index;
    };

    QVector<WordRange> m_words;
    QSet<int> m_revealedWordIndexes;
    QSet<int> m_usedButtonIndexes;

    QPointer<ContentModel> m_currentModel;
    QMetaObject::Connection m_titleConnection;
    QMetaObject::Connection m_paragraphsConnection;
    QMetaObject::Connection m_languageConnection;
    bool m_loading = false;
    QString m_errorString;
    QNetworkAccessManager m_networkManager;
    QNetworkReply *m_currentReply = nullptr;
};
