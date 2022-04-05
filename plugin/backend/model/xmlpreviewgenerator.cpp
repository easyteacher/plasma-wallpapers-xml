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

    QPixmap preview;

    if (!m_item.slideshow.data.empty() || QFile::exists(m_item.filename_dark)) {
        // Slideshow mode
        preview = generateSlideshowPreview();
    } else {
        preview = generateSinglePreview();
    }

    if (preview.isNull()) {
        Q_EMIT failed(m_item);
        return;
    }

    Q_EMIT gotPreview(m_item, preview);
}

QPixmap XmlPreviewGenerator::generateSinglePreview()
{
    QPixmap pix;
    QImage thumb(m_item.filename);

    if (thumb.isNull()) {
        return pix;
    }

    if (thumb.width() > m_screenshotSize.width() * thumb.devicePixelRatio() || thumb.height() > m_screenshotSize.height() * thumb.devicePixelRatio()) {
        pix = QPixmap::fromImage(thumb.scaled(m_screenshotSize * thumb.devicePixelRatio(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        pix = QPixmap::fromImage(thumb);
    }

    return pix;
}

QPixmap XmlPreviewGenerator::generateSlideshowPreview()
{
    int staticCount = 0;

    QVector<QImage> list;
    list.reserve(std::max<int>(m_item.slideshow.data.size(), 2));

    if (!m_item.slideshow.data.empty()) {
        for (const auto &item : std::as_const(m_item.slideshow.data)) {
            if (item.dataType == 0) {
                const QImage image(item.file);

                if (image.isNull()) {
                    continue;
                }

                if (m_screenshotSize.width() > m_screenshotSize.height()) {
                    list.append(image.scaledToHeight(m_screenshotSize.height(), Qt::SmoothTransformation));
                } else {
                    list.append(image.scaledToWidth(m_screenshotSize.width(), Qt::SmoothTransformation));
                }

                staticCount += 1;
            }
        }
    } else {
        if (m_screenshotSize.width() > m_screenshotSize.height()) {
            list.append(QImage(m_item.filename).scaledToHeight(m_screenshotSize.height(), Qt::SmoothTransformation));
            list.append(QImage(m_item.filename_dark).scaledToHeight(m_screenshotSize.height(), Qt::SmoothTransformation));
        } else {
            list.append(QImage(m_item.filename).scaledToWidth(m_screenshotSize.width(), Qt::SmoothTransformation));
            list.append(QImage(m_item.filename_dark).scaledToWidth(m_screenshotSize.width(), Qt::SmoothTransformation));
        }

        staticCount = 2;
    }

    if (staticCount == 0) {
        return QPixmap();
    }

    QPixmap pix(list.constFirst().size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);

    for (int i = 0; i < list.size(); i++) {
        const QImage &image = list.at(i);

        double start = i / static_cast<double>(list.size()), end = (i + 1) / static_cast<double>(list.size());

        QPoint topLeft(round(start * image.width()), 0);
        QPoint bottomRight(round(end * image.width()), image.height());
        QPoint topLeft2(round(start * pix.width()), 0);
        QPoint bottomRight2(round(end * pix.width()), pix.height());

        p.drawImage(QRect(topLeft2, bottomRight2), image.copy(QRect(topLeft, bottomRight)));
    }

    return pix;
}
