/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imageplugin.h"

#include <QQmlContext>

#include "backend/imagebackend.h"
#include "backend/slideshowbackend.h"
#include "backend/model/sortingmode.h"
#include "backend/model/imageproxymodel.h"
#include "backend/model/slidefiltermodel.h"
#include "backend/provider/xmlimageprovider.h"
#include "backend/provider/packageimageprovider.h"

void ImagePlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(uri == QLatin1String("com.github.easyteacher.plasma.wallpapers.xml"));

    engine->addImageProvider(QStringLiteral("gnome-wp-list"), new XmlImageProvider);
    engine->addImageProvider(QStringLiteral("package"), new PackageImageProvider);
}

void ImagePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("com.github.easyteacher.plasma.wallpapers.xml"));

    qmlRegisterType<ImageBackend>(uri, 1, 0, "ImageBackend");
    qmlRegisterType<SlideshowBackend>(uri, 1, 0, "SlideshowBackend");
    qmlRegisterAnonymousType<ImageProxyModel>("ImageProxyModel", 1);
    qmlRegisterAnonymousType<SlideFilterModel>("SlideFilterModel", 1);
    qmlRegisterUncreatableType<SortingMode>(uri, 1, 0, "SortingMode", "Error: only enums");
}
