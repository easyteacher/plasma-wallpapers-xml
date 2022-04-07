/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imagefinder.h"

#include <QCollator>
#include <QDir>
#include <QMimeDatabase>
#include <QImageReader>
#include <QSet>

#include "findsymlinktarget.h"

static QStringList s_suffixes;

ImageFinder::ImageFinder(const QStringList &paths, QObject *parent)
    : QObject(parent)
    , m_paths(paths)
{
}

void ImageFinder::run()
{
    QStringList images;

    QDir dir;
    dir.setFilter(QDir::AllDirs | QDir::Files | QDir::Readable);
    dir.setNameFilters(suffixes());

    int i;
    for (i = 0; i < m_paths.size(); ++i) {
        const QString &path = m_paths.at(i);
        const QString target = findSymlinkTarget(path);
        const QFileInfo info(target);

        if (!info.exists()) {
            continue;
        }

        if (info.baseName() == QLatin1String("screenshot") || target.contains(QLatin1String("contents/images"))) {
            // is a package
            continue;
        }

        if (suffixes().contains(QStringLiteral("*.%1").arg(info.suffix().toLower())) && info.isFile()) {
            images.append(target);
            continue;
        }

        dir.setPath(path);
        const QFileInfoList files = dir.entryInfoList();

        for (const QFileInfo &wp : files) {
            if (QString t(findSymlinkTarget(wp)); wp.isFile() && !t.contains(QLatin1String("contents/images")) && wp.baseName() != QLatin1String("screenshot")) {
                images.append(t);
            } else {
                const QString name = wp.fileName();

                if (name.startsWith('.')) {
                    continue;
                }

                // add this to the directories we should be looking at
                m_paths.append(wp.filePath());
            }
        }
    }

    images.removeAll(QString());
    images.removeDuplicates();

    sort(images);

    Q_EMIT imageFound(images);
}

void ImageFinder::sort(QStringList &list) const
{
    QCollator collator;

    // Make sure 2 comes before 10
    collator.setNumericMode(true);
    // Behave like Dolphin with natural sorting enabled
    collator.setCaseSensitivity(Qt::CaseInsensitive);

    const auto compare = [&collator](const QString &a, const QString &b) {
        // Checking if less than zero makes ascending order (A-Z)
        return collator.compare(a, b) < 0;
    };

    std::stable_sort(list.begin(), list.end(), compare);
}

QStringList suffixes()
{
    if (s_suffixes.isEmpty()) {
        QSet<QString> suffixes;

        QMimeDatabase db;
        const auto supportedMimeTypes = QImageReader::supportedMimeTypes();
        for (const QByteArray &mimeType : supportedMimeTypes) {
            QMimeType mime(db.mimeTypeForName(mimeType));
            const QStringList globPatterns = mime.globPatterns();
            for (const QString &pattern : globPatterns) {
                suffixes.insert(pattern);
            }
        }

        s_suffixes = suffixes.values();
    }

    return s_suffixes;
}
