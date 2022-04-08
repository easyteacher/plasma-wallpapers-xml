/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imageplugin.h"

#include <QQmlContext>

#include "backend/imagebackend.h"
#include "backend/slideshowbackend.h"
#include "model/imageproxymodel.h"
#include "model/slidefiltermodel.h"
#include "model/sortingmode.h"
#include "provider/packageimageprovider.h"
#include "provider/xmlimageprovider.h"

const auto pluginName = QByteArrayLiteral("com.github.easyteacher.plasma.wallpapers.xml");

void ImagePlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(uri == pluginName);

    engine->addImageProvider(QStringLiteral("gnome-wp-list"), new XmlImageProvider);
    engine->addImageProvider(QStringLiteral("package"), new PackageImageProvider);
}

void ImagePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == pluginName);

    qmlRegisterType<ImageBackend>(uri, 2, 0, "ImageBackend");
    qmlRegisterType<SlideshowBackend>(uri, 2, 0, "SlideshowBackend");

    qmlRegisterAnonymousType<ImageProxyModel>("ImageProxyModel", 2);
    qmlRegisterAnonymousType<SlideFilterModel>("SlideFilterModel", 2);

    qmlRegisterUncreatableType<SortingMode>(uri, 2, 0, "SortingMode", "Error: only enums");
}
