/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "packagelistmodel.h"

#include <QDir>
#include <QPixmap>
#include <QThreadPool>

#include <KAboutData>
#include <KConfigGroup>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KSharedConfig>

#include "../finder/packagefinder.h"

PackageListModel::PackageListModel(const QStringList &customPaths, const QSize &targetSize, QObject* parent)
    : AbstractImageListModel(customPaths, targetSize, parent)
{
    qRegisterMetaType<QList<KPackage::Package>>();
    load(customPaths);
}

int PackageListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_packages.size();
}

QVariant PackageListModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();

    if (!index.isValid() || row >= m_packages.size() || row < 0) {
        return QVariant();
    }

    const KPackage::Package &b = m_packages.at(row);

    if (!b.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return packageDisplayName(b);

    case ScreenshotRole: {
        const QString path = b.filePath("preferred");

        QPixmap *cachedPreview = m_imageCache.object(path);

        if (cachedPreview) {
            return *cachedPreview;
        }

        asyncGetPreview(path, QPersistentModelIndex(index));

        return QVariant();
    }

    case AuthorRole: {
        if (!b.metadata().authors().isEmpty()) {
            return b.metadata().authors().constFirst().name();
        }

        return QString();
    }

    case ResolutionRole: {
        const QString path = b.filePath("preferred");

        QSize *size = m_imageSizeCache.object(path);

        if (size && size->isValid()) {
            return QStringLiteral("%1x%2").arg(size->width(), size->height());
        }

        asyncGetImageSize(path, QPersistentModelIndex(index));

        return QString();
    }

    case PathRole:
        return QUrl::fromLocalFile(b.filePath("preferred"));

    case PackageNameRole:
        return b.path();

    case RemovableRole: {
        const QString path = b.path();

        return path.startsWith(s_localImageDir) || m_removableWallpapers.contains(path);
    }

    case PendingDeletionRole:
        return m_pendingDeletion.value(b.path(), false);

    default:
        return QVariant();
    }
}

bool PackageListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    if (role == PendingDeletionRole) {
        const KPackage::Package &b = m_packages.at(index.row());
        m_pendingDeletion[b.path()] = value.toBool();

        Q_EMIT dataChanged(index, index, {PendingDeletionRole});
        return true;
    }

    return false;
}

int PackageListModel::indexOf(const QString &path) const
{
    const auto it = std::find_if(m_packages.cbegin(), m_packages.cend(), [&path](const KPackage::Package &p) {
        return path == p.path();
    });

    if (it == m_packages.cend()) {
        return -1;
    }

    return std::distance(m_packages.cbegin(), it);
}

void PackageListModel::load(const QStringList &customPaths)
{
    if (m_loading) {
        return;
    }

    for (const auto &p : std::as_const(m_packages)) {
        m_dirWatch.removeDir(p.path());
    }

    if (customPaths.empty()) {
        KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("plasmarc")), QStringLiteral("Wallpapers"));
        m_customPaths = cfg.readEntry("usersWallpapers", QStringList{});

        m_customPaths += QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("wallpapers/"), QStandardPaths::LocateDirectory);
    } else {
        m_customPaths = customPaths;
    }

    m_customPaths.removeDuplicates();

    PackageFinder *finder = new PackageFinder(m_customPaths, m_targetSize);
    connect(finder, &PackageFinder::packageFound, this, &PackageListModel::slotHandlePackageFound);
    QThreadPool::globalInstance()->start(finder);

    m_loading = true;
}

QStringList PackageListModel::addBackground(const QString &path)
{
    if (path.isEmpty() || indexOf(path) >= 0 || !QFileInfo(path).isDir()) {
        return {};
    }

    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Wallpaper/Images"));
    package.setPath(path);

    if (!package.isValid() || !package.metadata().isValid()) {
        return {};
    }

    findPreferredImageInPackage(package, m_targetSize);

    beginInsertRows(QModelIndex(), 0, 0);

    m_removableWallpapers.prepend(package.path());
    m_packages.prepend(package);

    if (!m_dirWatch.contains(path)) {
        m_dirWatch.addDir(path);
    }

    endInsertRows();
    Q_EMIT countChanged();

    return {path};
}

void PackageListModel::removeBackground(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    int idx = indexOf(path);

    if (idx < 0) {
        return;
    }

    beginRemoveRows(QModelIndex(), idx, idx);

    m_pendingDeletion.remove(m_packages.at(idx).path());
    m_removableWallpapers.removeOne(m_packages.at(idx).path());
    m_packages.removeAt(idx);

    endRemoveRows();
    Q_EMIT countChanged();

    // Uninstall local package
    if (!path.startsWith(s_localImageDir)) {
        return;
    }

    const QString packageName = path.split(QDir::separator(), Qt::SkipEmptyParts).constLast();

    if (packageName == QLatin1String("wallpapers")) {
        return;
    }

    KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Wallpaper/Images"));
    p.uninstall(packageName, s_localImageDir);
}

void PackageListModel::slotHandlePackageFound(const QList<KPackage::Package> &packages)
{
    beginResetModel();

    m_packages = packages;

    for (const auto &p : packages) {
        if (!m_dirWatch.contains(p.path())) {
            m_dirWatch.addDir(p.path());
        }
    }

    endResetModel();

    Q_EMIT countChanged();
    m_loading = false;
    Q_EMIT loaded(this);
}

