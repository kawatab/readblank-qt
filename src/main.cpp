// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "controller/ContentController.h"
#include "model/FavoriteListModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName("WITHLET11");
    QCoreApplication::setApplicationName("ReadBlank Qt");

    FavoriteListModel favoriteListModel;
    ContentController contentController;

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty(
        "favoriteListModel", &favoriteListModel);

    engine.rootContext()->setContextProperty(
        "contentController", &contentController);

    engine.loadFromModule("ReadBlankQt", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
