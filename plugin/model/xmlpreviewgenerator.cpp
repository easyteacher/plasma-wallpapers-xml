/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xmlpreviewgenerator.h"

#include <math.h>

#include <QFile>
#include <QPainter>

XmlPreviewGenerator::XmlPreviewGenerator(const WallpaperItem &item, const QSize &size, QObject *parent)
    : QObject(parent)
    , m_item(item)
    , m_screenshotSize(size)
{
}

void XmlPreviewGenerator::run()
{
    if (!QFile::exists(m_item.filename)) {
        // At least the light wallpaper must be available
        Q_EMIT failed(m_item);
        return;
    }

    QPixmap *preview = nullptr;

    if (!m_item.slideshow.data.empty() || QFile::exists(m_item.filename_dark)) {
        // Slideshow mode
        preview = generateSlideshowPreview();
    } else {
        preview = generateSinglePreview();
    }

    if (!preview || preview->isNull()) {
        delete preview;
        Q_EMIT failed(m_item);
        return;
    }

    Q_EMIT gotPreview(m_item, preview);
}

QPixmap *XmlPreviewGenerator::generateSinglePreview()
{
    QPixmap *pix = new QPixmap(m_item.filename);

    if (pix->isNull()) {
        return pix;
    }

    if (pix->width() > m_screenshotSize.width() * pix->devicePixelRatio() || pix->height() > m_screenshotSize.height() * pix->devicePixelRatio()) {
        auto resizedPix = new QPixmap(pix->scaled(m_screenshotSize * pix->devicePixelRatio(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        delete pix;
        return resizedPix;
    }

    return pix;
}

QPixmap *XmlPreviewGenerator::generateSlideshowPreview()
{
    int staticCount = 0;

    std::vector<QImage> list;
    list.reserve(std::max<int>(m_item.slideshow.data.size(), 2));

    if (!m_item.slideshow.data.empty()) {
        for (const auto &item : std::as_const(m_item.slideshow.data)) {
            if (item.dataType == 0) {
                const QImage image(item.file);

                if (image.isNull()) {
                    continue;
                }

                if (m_screenshotSize.width() > m_screenshotSize.height()) {
                    list.emplace_back(image.scaledToHeight(m_screenshotSize.height(), Qt::SmoothTransformation));
                } else {
                    list.emplace_back(image.scaledToWidth(m_screenshotSize.width(), Qt::SmoothTransformation));
                }

                staticCount += 1;
            }
        }
    } else {
        if (m_screenshotSize.width() > m_screenshotSize.height()) {
            list.emplace_back(QImage(m_item.filename).scaledToHeight(m_screenshotSize.height(), Qt::SmoothTransformation));
            list.emplace_back(QImage(m_item.filename_dark).scaledToHeight(m_screenshotSize.height(), Qt::SmoothTransformation));
        } else {
            list.emplace_back(QImage(m_item.filename).scaledToWidth(m_screenshotSize.width(), Qt::SmoothTransformation));
            list.emplace_back(QImage(m_item.filename_dark).scaledToWidth(m_screenshotSize.width(), Qt::SmoothTransformation));
        }

        staticCount = 2;
    }

    if (staticCount == 0) {
        return nullptr;
    }

    QPixmap *pix = new QPixmap(list.at(0).size());
    pix->fill(Qt::black);
    auto p = QScopedPointer(new QPainter(pix));

    for (int i = 0; i < list.size(); i++) {
        const QImage &image = list.at(i);

        double start = i / static_cast<double>(list.size()), end = (i + 1) / static_cast<double>(list.size());

        QPoint topLeft(round(start * image.width()), 0);
        QPoint bottomRight(round(end * image.width()), image.height());
        QPoint topLeft2(round(start * pix->width()), 0);
        QPoint bottomRight2(round(end * pix->width()), pix->height());

        p->drawImage(QRect(topLeft2, bottomRight2), image.copy(QRect(topLeft, bottomRight)));
    }

    p->end();

    return pix;
}
