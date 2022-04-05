/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "packagefinder.h"

#include <QCollator>
#include <QDir>

#include <KLocalizedString>
#include <KPackage/PackageLoader>

#include "distance.h"
#include "findsymlinktarget.h"

PackageFinder::PackageFinder(const QStringList &paths, const QSize &targetSize, QObject *parent)
    : QObject(parent)
    , m_paths(paths)
    , m_targetSize(targetSize)
{
}

void PackageFinder::run()
{
    QList<KPackage::Package> packages;
    QStringList folders;

    QDir dir;
    dir.setFilter(QDir::AllDirs | QDir::Readable);
    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Wallpaper/Images"));

    int i;
    for (i = 0; i < m_paths.count(); ++i) {
        const QString &path = m_paths.at(i);
        const QFileInfo info(path);

        if (!info.exists() || info.isFile()) {
            continue;
        }

        dir.setPath(path);
        const QFileInfoList files = dir.entryInfoList();

        for (const QFileInfo &wp : files) {
            if (wp.isFile()) {
                // Package is inside a directory
                continue;
            }

            const QString name = wp.fileName();

            if (name.startsWith('.')) {
                continue;
            }

            const QString folderPath = findSymlinkTarget(wp);

            if (folders.contains(folderPath)) {
                continue;
            }

            if (QFile::exists(folderPath + QLatin1String("/metadata.desktop")) || QFile::exists(folderPath + QLatin1String("/metadata.json"))) {
                package.setPath(folderPath);

                if (package.isValid() && package.metadata().isValid() && !package.filePath("images").isEmpty()) {
                    findPreferredImageInPackage(package, m_targetSize);
                    packages << package;
                    folders << folderPath;
                }

                continue;
            }

            // add this to the directories we should be looking at
            m_paths.append(folderPath);
        }
    }

    Q_EMIT packageFound(packages);
}

void PackageFinder::sort (QList<KPackage::Package> &list) const
{
    QCollator collator;

    // Make sure 2 comes before 10
    collator.setNumericMode(true);
    // Behave like Dolphin with natural sorting enabled
    collator.setCaseSensitivity(Qt::CaseInsensitive);

    const auto compare = [&collator](const KPackage::Package &a, const KPackage::Package &b) {
        // Checking if less than zero makes ascending order (A-Z)
        return collator.compare(packageDisplayName(a), packageDisplayName(b)) < 0;
    };

    std::stable_sort(list.begin(), list.end(), compare);
}

void findPreferredImageInPackage(KPackage::Package &package, const QSize &targetSize)
{
    if (!package.isValid() || !package.filePath("preferred").isEmpty()) {
        return;
    }

    QSize tSize = targetSize;

    if (tSize.isEmpty()) {
        tSize = QSize(1920, 1080);
    }

    // find preferred size
    QString preferred;
    {
        const QStringList images = package.entryList("images");

        if (images.empty()) {
            return;
        }

        float best = std::numeric_limits<float>::max();

        for (const QString &entry : images) {
            QSize candidate = resSize(QFileInfo(entry).baseName());

            if (candidate == QSize()) {
                continue;
            }

            float dist = distance(candidate, tSize);

            if (preferred.isEmpty() || dist < best) {
                preferred = entry;
                best = dist;
            }
        }
    }

    package.removeDefinition("preferred");
    package.addFileDefinition("preferred", QStringLiteral("images/") + preferred, i18n("Recommended wallpaper file"));
}

QString packageDisplayName(const KPackage::Package &b)
{
    QString title = b.metadata().isValid() ? b.metadata().name() : QString();

    if (title.isEmpty()) {
        return QFileInfo(b.filePath("preferred")).completeBaseName();
    }

    return title;
}
