// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QUrl>

#include "ContentModel.h"

class FavoriteListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FavoriteListModel(QObject *parent = nullptr);

    enum Roles { UrlRole = Qt::UserRole + 1, TitleRole, LanguageRole, LastAccessedRole, ContentModelRole };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QObject *firstItem() const;
    Q_INVOKABLE void addUrl(const QUrl &url);
    Q_INVOKABLE void addUrlFromClipboard();
    Q_INVOKABLE void moveToTop(int row);
    Q_INVOKABLE void removeUrl(int row);

private:
    void load();
    void save();
    void connectItem(ContentModel *item);

    QList<ContentModel *> m_items;
};
