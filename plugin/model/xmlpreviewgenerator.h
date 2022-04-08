/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XMLPREVIEWGENERATOR_H
#define XMLPREVIEWGENERATOR_H

#include <QPixmap>

#include "../finder/xmlfinder.h"

/**
 * @todo write docs
 */
class XmlPreviewGenerator : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit XmlPreviewGenerator(const WallpaperItem &item, const QSize &size, QObject *parent = nullptr);

    void run() override;

Q_SIGNALS:
    void gotPreview(const WallpaperItem &item, QPixmap *preview);
    void failed(const WallpaperItem &item);

private:
    QPixmap *generateSinglePreview();
    QPixmap *generateSlideshowPreview();

    WallpaperItem m_item;
    QSize m_screenshotSize;
};

#endif // XMLPREVIEWGENERATOR_H
