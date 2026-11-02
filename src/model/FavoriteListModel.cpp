// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#include "FavoriteListModel.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QSettings>
#include <QStringList>

FavoriteListModel::FavoriteListModel(QObject *parent) : QAbstractListModel(parent)
{
    load();
}

int FavoriteListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

QVariant FavoriteListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return {};
    }

    ContentModel *item = m_items.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case UrlRole:
        return item->url();

    case TitleRole:
        return item->title();

    case LanguageRole:
        return item->language();

    case LastAccessedRole:
        return item->lastAccessed();

    case ContentModelRole:
        return QVariant::fromValue(static_cast<QObject *>(item));

    default:
        return {};
    }
}

QHash<int, QByteArray> FavoriteListModel::roleNames() const
{
    return {
        { UrlRole, "url" }, { TitleRole, "title" }, { LanguageRole, "language" },
        { LastAccessedRole, "lastAccessed" },
        { ContentModelRole, "contentModel" } };
}

QObject *FavoriteListModel::firstItem() const
{
    if (m_items.isEmpty())
        return nullptr;

    return m_items.first();
}

void FavoriteListModel::addUrl(const QUrl &url)
{
    if (!url.isValid() || url.isEmpty())
        return;

    auto *item = new ContentModel(this);
    item->setUrl(url.toString());

    connectItem(item);

    const int row = m_items.size();

    beginInsertRows(QModelIndex(), row, row);
    m_items.append(item);
    endInsertRows();

    save();
}

void FavoriteListModel::addUrlFromClipboard()
{
    const QString text = QGuiApplication::clipboard()->text().trimmed();

    const QUrl url = QUrl::fromUserInput(text);

    if (!url.isValid() || url.isEmpty())
        return;

    addUrl(url);
}
void FavoriteListModel::moveToTop(int row)
{
    if (row <= 0 || row >= m_items.size())
        return;

    if (!beginMoveRows(QModelIndex(), row, row, QModelIndex(), 0)) {
        return;
    }

    ContentModel *item = m_items.takeAt(row);
    m_items.prepend(item);

    endMoveRows();

    save();
}

void FavoriteListModel::removeUrl(int row)
{
    if (row < 0 || row >= m_items.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);

    ContentModel *item = m_items.takeAt(row);

    endRemoveRows();

    delete item;

    save();
}

void FavoriteListModel::connectItem(ContentModel *item)
{
    auto notifyItemChanged = [this, item](int role) {
        const int row = m_items.indexOf(item);

        if (row < 0)
            return;

        const QModelIndex modelIndex = index(row, 0);

        emit dataChanged(modelIndex, modelIndex, { role });

        save();
    };

    connect(item, &ContentModel::urlChanged, this,
            [notifyItemChanged]() { notifyItemChanged(UrlRole); });

    connect(item, &ContentModel::titleChanged, this,
            [notifyItemChanged]() { notifyItemChanged(TitleRole); });

    connect(item, &ContentModel::languageChanged, this,
            [notifyItemChanged]() { notifyItemChanged(LanguageRole); });
}

void FavoriteListModel::load()
{
    QSettings settings;

    const int size = settings.beginReadArray("favoriteUrls");

    m_items.clear();
    m_items.reserve(size);

    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        const QUrl url(settings.value("url").toString());

        if (!url.isValid() || url.isEmpty())
            continue;

        auto *item = new ContentModel(this);

        item->setUrl(url.toString());

        item->setMetadata(settings.value("title").toString(),
                          settings.value("language").toString());

        item->setLastAccessed(settings.value("lastAccessed").toDateTime());

        connectItem(item);

        m_items.append(item);
    }

    settings.endArray();
}

void FavoriteListModel::save()
{
    QSettings settings;

    settings.beginWriteArray("favoriteUrls");

    for (int i = 0; i < m_items.size(); ++i) {
        ContentModel *item = m_items.at(i);

        settings.setArrayIndex(i);
        settings.setValue("url", item->url());
        settings.setValue("title", item->title());
        settings.setValue("language", item->language());
        settings.setValue("lastAccessed", item->lastAccessed());
    }

    settings.endArray();
    settings.sync();
}
