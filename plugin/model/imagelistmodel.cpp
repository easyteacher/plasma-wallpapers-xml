/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imagelistmodel.h"

#include <QFileInfo>
#include <QPixmap>
#include <QThreadPool>
#include <QUrl>

#include <KConfigGroup>
#include <KFileItem>
#include <KIO/PreviewJob>
#include <KSharedConfig>

#include "../finder/imagefinder.h"

ImageListModel::ImageListModel(const QStringList &customPaths, const QSize &targetSize, QObject *parent)
    : AbstractImageListModel(customPaths, targetSize, parent)
{
    connect(&m_dirWatch, &KDirWatch::created, this, &ImageListModel::addBackground);
    load(customPaths);
}

int ImageListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_data.size();
}

QVariant ImageListModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();

    if (!index.isValid() || row >= m_data.size() || row < 0) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return QFileInfo(m_data.at(row)).fileName();

    case ScreenshotRole: {
        QPixmap *cachedPreview = m_imageCache.object(m_data.at(row));

        if (cachedPreview) {
            return *cachedPreview;
        }

        asyncGetPreview(m_data.at(row), QPersistentModelIndex(index));

        return QVariant();
    }

    case AuthorRole:
        // No author for single image file
        return QString();

    case ResolutionRole: {
        QSize *size = m_imageSizeCache.object(m_data.at(row));

        if (size && size->isValid()) {
            return QString::fromLatin1("%1x%2").arg(size->width()).arg(size->height());;
        }

        asyncGetImageSize(m_data.at(row), QPersistentModelIndex(index));

        return QString();
    }

    case PathRole:
        return QUrl::fromLocalFile(m_data.at(row));

    case PackageNameRole:
        return m_data.at(row);

    case RemovableRole: {
        const QString &path = m_data.at(row);

        return path.startsWith(s_localImageDir) || m_removableWallpapers.contains(path);
    }

    case PendingDeletionRole:
        return m_pendingDeletion.value(m_data.at(row), false);

    default:
        return QVariant();
    }
}

bool ImageListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    if (role == PendingDeletionRole) {
        m_pendingDeletion[m_data.at(index.row())] = value.toBool();

        Q_EMIT dataChanged(index, index, {PendingDeletionRole});
        return true;
    }

    return false;
}

int ImageListModel::indexOf(const QString &path) const
{
    const auto it = std::find_if(m_data.cbegin(), m_data.cend(), [&path](const QString &p) {
        return path == p;
    });

    if (it == m_data.cend()) {
        return -1;
    }

    return std::distance(m_data.cbegin(), it);
}

void ImageListModel::load(const QStringList &customPaths)
{
    if (m_loading) {
        return;
    }

    for (const auto &p : std::as_const(m_data)) {
        m_dirWatch.removeFile(p);
    }

    for (const auto &p : std::as_const(m_customPaths)) {
        m_dirWatch.removeDir(p);
    }

    if (customPaths.empty()) {
        KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("plasmarc")), QStringLiteral("Wallpapers"));
        m_removableWallpapers = cfg.readEntry("usersWallpapers", QStringList{});

        m_customPaths = m_removableWallpapers + QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("wallpapers/"), QStandardPaths::LocateDirectory);
    } else {
        m_customPaths = customPaths;
    }

    m_customPaths.removeDuplicates();

    ImageFinder *finder = new ImageFinder(m_customPaths);
    connect(finder, &ImageFinder::imageFound, this, &ImageListModel::slotHandleImageFound);
    QThreadPool::globalInstance()->start(finder);

    m_loading = true;
}

void ImageListModel::slotHandleImageFound(const QStringList &paths)
{
    beginResetModel();

    m_data = paths;

    for (const QString &p : paths) {
        if (!m_dirWatch.contains(p)) {
            m_dirWatch.addFile(p);
        }
    }

    for (const QString &p : std::as_const(m_customPaths)) {
        if (!m_dirWatch.contains(p) && QFileInfo(p).isDir()) {
            m_dirWatch.addDir(p, KDirWatch::WatchFiles);
        }
    }

    endResetModel();
    Q_EMIT countChanged();

    m_loading = false;
    Q_EMIT loaded(this);
}

QStringList ImageListModel::addBackground(const QString &path)
{
    if (path.isEmpty() || !QFile::exists(path) || m_data.contains(path)) {
        return {};
    }


    if (!suffixes().contains(QStringLiteral("*.%1").arg(QFileInfo(path).suffix()))) {
        // Format not supported
        return {};
    }

    beginInsertRows(QModelIndex(), 0, 0);

    m_data.prepend(path);
    m_removableWallpapers.prepend(path);

    if (!m_dirWatch.contains(path)) {
        m_dirWatch.addFile(path);
    }

    endInsertRows();
    Q_EMIT countChanged();

    return {path};
}

void ImageListModel::removeBackground(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    const int idx = indexOf(path);

    if (idx < 0) {
        return;
    }

    beginRemoveRows(QModelIndex(), idx, idx);

    m_pendingDeletion.remove(m_data.at(idx));
    m_removableWallpapers.removeOne(m_data.at(idx));
    m_data.removeAt(idx);

    m_dirWatch.removeFile(path);

    endRemoveRows();
    Q_EMIT countChanged();

    // Remove local wallpaper
    if (path.startsWith(s_localImageDir)) {
        QFile f(path);

        if (f.exists()) {
            f.remove();
        }
    }
}

