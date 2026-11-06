// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

#include "controller/ContentController.h"
#include "controller/LanguageController.h"
#include "controller/SpeechController.h"
#include "model/FavoriteListModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName("WITHLET11");
    QCoreApplication::setApplicationName("ReadBlank Qt");

    QTranslator translator;

    if (translator.load(QLocale(), "readblank-qt", "_", ":/i18n")) {
        app.installTranslator(&translator);
    }

    FavoriteListModel favoriteListModel;
    ContentController contentController;
    SpeechController speechController;

    QObject::connect(&contentController, &ContentController::currentParagraphChanged,
                     &speechController, &SpeechController::stop);

    QObject::connect(&contentController, &ContentController::currentModelChanged, &speechController,
                     &SpeechController::stop);

    QQmlApplicationEngine engine;

    LanguageController *languageController = new LanguageController(&engine);

    engine.rootContext()->setContextProperty("favoriteListModel", &favoriteListModel);
    engine.rootContext()->setContextProperty("contentController", &contentController);
    engine.rootContext()->setContextProperty("speechController", &speechController);
    engine.rootContext()->setContextProperty("languageController", languageController);

    engine.loadFromModule("ReadBlankQt", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
