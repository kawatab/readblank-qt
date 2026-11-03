// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#include "ContentModel.h"

ContentModel::ContentModel(QObject *parent)
    : QObject(parent)
{
}

QString ContentModel::url() const
{
    return m_url;
}

QString ContentModel::title() const
{
    return m_title;
}

QString ContentModel::language() const
{
    return m_language;
}

QDateTime ContentModel::lastAccessed() const
{
    return m_lastAccessed;
}

QStringList ContentModel::paragraphs() const
{
    return m_paragraphs;
}

QString ContentModel::content() const
{
    return m_paragraphs.join('\n');
}

bool ContentModel::loaded() const
{
    return m_loaded;
}

void ContentModel::setUrl(const QString &url)
{
    if (m_url == url)
        return;

    m_url = url;
    emit urlChanged();
}

void ContentModel::setTitle(const QString &title)
{
    if (m_title == title)
        return;

    m_title = title;
    emit titleChanged();
}

void ContentModel::setLanguage(const QString &language)
{
    if (m_language == language)
        return;

    m_language = language;
    emit languageChanged();
}

void ContentModel::setLastAccessed(const QDateTime &dateTime)
{
    if (m_lastAccessed == dateTime)
        return;

    m_lastAccessed = dateTime;
    emit lastAccessedChanged();
}

void ContentModel::setMetadata(
    const QString &title,
    const QString &language)
{
    setTitle(title);
    setLanguage(language);
}

void ContentModel::setParagraphs(const QStringList &paragraphs)
{
    const bool paragraphsChanged = m_paragraphs != paragraphs;
    const bool loadedChangedValue = !m_loaded;

    if (!paragraphsChanged && !loadedChangedValue)
        return;

    m_paragraphs = paragraphs;
    m_loaded = true;

    if (paragraphsChanged)
        emit contentChanged();

    if (loadedChangedValue)
        emit loadedChanged();
}

void ContentModel::clearCache()
{
    if (!m_loaded)
        return;

    m_paragraphs.clear();
    m_loaded = false;

    emit contentChanged();
    emit loadedChanged();
}
