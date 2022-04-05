/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xmlimagelistmodel.h"

#include <QFile>
#include <QPixmap>
#include <QThreadPool>
#include <QUrlQuery>

#include <KConfigGroup>
#include <KSharedConfig>

#include "../finder/xmlfinder.h"
#include "xmlpreviewgenerator.h"

XmlImageListModel::XmlImageListModel(const QStringList &customPaths, const QSize &targetSize, QObject *parent)
    : AbstractImageListModel(customPaths, targetSize, parent)
{
    qRegisterMetaType<WallpaperItem>();
    qRegisterMetaType<QList<WallpaperItem>>();
    load(customPaths);
}

int XmlImageListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_data.size();
}

QVariant XmlImageListModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();

    if (!index.isValid() || row >= m_data.size() || row < 0) {
        return QVariant();
    }

    const auto &item = m_data.at(row);

    switch (role) {
    case Qt::DisplayRole:
        return item.name;

    case ScreenshotRole: {
        QPixmap *cachedPreview = m_imageCache.object(item.path.toString());

        if (cachedPreview) {
            return *cachedPreview;
        }

        asyncGetXmlPreview(item, QPersistentModelIndex(index));

        return QVariant();
    }

    case AuthorRole:
        return item.author;

    case ResolutionRole: {
        QSize *size = m_imageSizeCache.object(item.path.toString());

        if (size && size->isValid()) {
            return QString::fromLatin1("%1x%2").arg(size->width()).arg(size->height());;
        }

        asyncGetImageSize(item.path.toString(), QPersistentModelIndex(index));

        return QString();
    }

    case PathRole:
        return QUrl::fromLocalFile(item.filename);

    case PackageNameRole:
        return item.path.toString();

    case RemovableRole: {
        return m_removableWallpapers.contains(item.path.toString());
    }

    case PendingDeletionRole:
        return m_pendingDeletion.value(item.path.toString(), false);

    default:
        return QVariant();
    }
}

bool XmlImageListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    if (role == PendingDeletionRole) {
        m_pendingDeletion[m_data.at(index.row()).path.toString()] = value.toBool();

        Q_EMIT dataChanged(index, index, {PendingDeletionRole});
        return true;
    }

    return false;
}

int XmlImageListModel::indexOf(const QString &path) const
{
    const auto it = std::find_if(m_data.cbegin(), m_data.cend(), [&path](const WallpaperItem &p) {
        return path == p.path.toString();
    });

    if (it == m_data.cend()) {
        return -1;
    }

    return std::distance(m_data.cbegin(), it);
}

void XmlImageListModel::load(const QStringList &customPaths)
{
    if (m_loading) {
        return;
    }

    for (const auto &p : std::as_const(m_data)) {
        m_dirWatch.removeFile(p._root);
        m_dirWatch.removeFile(p.filename);
    }

    if (customPaths.empty()) {
        const KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("plasmarc")), QStringLiteral("Wallpapers"));
        m_removableWallpapers = cfg.readEntry("usersWallpapers", QStringList{});

        m_customPaths = m_removableWallpapers + QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("gnome-background-properties/"), QStandardPaths::LocateDirectory);
    } else {
        m_customPaths = customPaths;
    }

    m_customPaths.removeDuplicates();

    XmlFinder *finder = new XmlFinder(m_customPaths, m_targetSize);
    connect(finder, &XmlFinder::xmlFound, this, &XmlImageListModel::slotHandleXmlFound);
    QThreadPool::globalInstance()->start(finder);

    m_loading = true;
}

QStringList XmlImageListModel::addBackground(const QString &path)
{
    QStringList results;

    if (!path.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive)) {
        return results;
    }

    const auto items = XmlFinder::parseXml(path, m_targetSize);

    if (items.empty()) {
        return results;
    }

    beginInsertRows(QModelIndex(), 0, items.size() - 1);

    for (const auto &p : items) {
        m_data.prepend(p);
        m_removableWallpapers.prepend(p.path.toString());
        results.prepend(p.path.toString());

        if (!m_dirWatch.contains(p._root)) {
            m_dirWatch.addFile(p._root);
        }

        if (!m_dirWatch.contains(p.filename)) {
            m_dirWatch.addFile(p.filename);
        }
    }

    endInsertRows();
    Q_EMIT countChanged();

    return results;
}

void XmlImageListModel::removeBackground(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    int idx = indexOf(path);

    if (idx < 0) {
        // Check root path
        const auto it = std::find_if(m_data.cbegin(), m_data.cend(), [&path](const WallpaperItem &p) {
            return p._root == path;
        });

        if (it != m_data.cend()) {
            idx = std::distance(m_data.cbegin(), it);
        }
    }

    if (idx < 0) {
        // Check filename
        const auto it2 = std::find_if(m_data.cbegin(), m_data.cend(), [&path](const WallpaperItem &p) {
            return p.filename == path;
        });

        if (it2 != m_data.cend()) {
            idx = std::distance(m_data.cbegin(), it2);
        }
    }

    if (idx < 0) {
        return;
    }

    beginRemoveRows(QModelIndex(), idx, idx);

    const auto &p = m_data.at(idx);
    m_pendingDeletion.remove(p.path.toString());
    m_removableWallpapers.removeOne(p.path.toString());

    m_dirWatch.removeFile(p._root);
    m_dirWatch.removeFile(p.filename);

    m_data.removeAt(idx);

    endRemoveRows();
    Q_EMIT countChanged();
}

void XmlImageListModel::slotHandleXmlFound(const QList<WallpaperItem> &packages)
{
    beginResetModel();

    m_data = packages;

    for (const auto &p : packages) {
        if (!m_dirWatch.contains(p._root)) {
            m_dirWatch.addFile(p._root);
        }

        if (!m_dirWatch.contains(p.filename)) {
            m_dirWatch.addFile(p.filename);
        }
    }

    endResetModel();

    Q_EMIT countChanged();
    m_loading = false;
    Q_EMIT loaded(this);
}

void XmlImageListModel::slotHandleXmlPreview(const WallpaperItem &item, const QPixmap &preview)
{
    const QPersistentModelIndex pidx = m_previewJobsUrls.take(item.path.toString());
    QModelIndex idx;

    if (!pidx.isValid()) {
        if (int row = indexOf(item.path.toString()); row >= 0) {
            idx = index(row, 0);
        } else {
            return;
        }
    } else {
        idx = pidx;
    }

    const int cost = preview.width() * preview.height() * preview.depth() / 8;

    if (m_imageCache.insert(item.path.toString(), new QPixmap(preview), cost)) {
        Q_EMIT dataChanged(idx, idx, {ScreenshotRole});
    }
}

void XmlImageListModel::slotHandleXmlPreviewFailed(const WallpaperItem &item)
{
    m_previewJobsUrls.remove(item.path.toString());
}

void XmlImageListModel::asyncGetXmlPreview(const WallpaperItem& item, const QPersistentModelIndex& index) const
{
    if (m_previewJobsUrls.contains(item.path.toString()) || item.path.isEmpty()) {
        return;
    }

    XmlPreviewGenerator *finder = new XmlPreviewGenerator(item, m_screenshotSize);
    connect(finder, &XmlPreviewGenerator::gotPreview, this, &XmlImageListModel::slotHandleXmlPreview);
    connect(finder, &XmlPreviewGenerator::failed, this, &XmlImageListModel::slotHandleXmlPreviewFailed);
    QThreadPool::globalInstance()->start(finder);

    m_previewJobsUrls.insert(item.path.toString(), index);
}
