// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#include "LanguageController.h"

#include <QCoreApplication>
#include <QLocale>
#include <QSettings>
#include <QQmlEngine>

LanguageController::LanguageController(
    QQmlEngine *engine, QObject *parent)
    : QObject(parent),
      m_engine(engine)
{
    QSettings settings;
    m_language = settings.value("language", "").toString();

    loadLanguage();
}

QString LanguageController::language() const
{
    return m_language;
}

void LanguageController::setLanguage(const QString &language)
{
    if (m_language == language)
        return;

    m_language = language;

    QSettings settings;
    settings.setValue("language", m_language);

    loadLanguage();

    emit languageChanged();
}

void LanguageController::loadLanguage()
{
    QCoreApplication::removeTranslator(&m_translator);

    QString localeName = m_language;

    if (localeName.isEmpty())
        localeName = QLocale::system().name();

    const QLocale locale(localeName);

    if (locale.language() != QLocale::English
        && m_translator.load(
            locale,
            "readblank-qt",
            "_",
            ":/i18n")) {
        QCoreApplication::installTranslator(&m_translator);
    }

    m_engine->retranslate();
}
