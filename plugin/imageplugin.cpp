/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imageplugin.h"
#include <QQmlContext>

#include <KFileItem>

#include "imagebackend.h"
#include "provider/packageimageprovider.h"
#include "provider/xmlimageprovider.h"
#include "sortingmode.h"

const auto pluginName = QByteArrayLiteral("com.github.easyteacher.plasma.wallpapers.xml");

void ImagePlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(uri == pluginName);

    engine->addImageProvider(QStringLiteral("package"), new PackageImageProvider);
    engine->addImageProvider(QStringLiteral("gnome-wp-list"), new XmlImageProvider);
}

void ImagePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == pluginName);

    qRegisterMetaType<KFileItem>(); // For image preview

    qmlRegisterType<ImageBackend>(uri, 2, 0, "ImageBackend");
    qmlRegisterAnonymousType<QAbstractItemModel>("QAbstractItemModel", 1);
    qmlRegisterUncreatableType<SortingMode>(uri, 2, 0, "SortingMode", "error: only enums");
}
