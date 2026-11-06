// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#pragma once

#include <QObject>
#include <QTranslator>

class QQmlEngine;

class LanguageController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString language READ language
               WRITE setLanguage NOTIFY languageChanged)

public:
    explicit LanguageController(QQmlEngine *engine,
                                QObject *parent = nullptr);

    QString language() const;
    void setLanguage(const QString &language);

signals:
    void languageChanged();

private:
    QQmlEngine *m_engine;
    QTranslator m_translator;
    QString m_language;

    void loadLanguage();
};
