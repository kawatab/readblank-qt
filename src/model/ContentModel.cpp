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

QString ContentModel::content() const
{
    return m_content;
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

void ContentModel::setContent(const QString &content)
{
    const bool contentChangedValue = m_content != content;
    const bool loadedChangedValue = !m_loaded;

    if (!contentChangedValue && !loadedChangedValue)
        return;

    m_content = content;
    m_loaded = true;

    if (contentChangedValue)
        emit contentChanged();

    if (loadedChangedValue)
        emit loadedChanged();
}

void ContentModel::clearCache()
{
    if (!m_loaded)
        return;

    m_content.clear();
    m_loaded = false;

    emit contentChanged();
    emit loadedChanged();
}
