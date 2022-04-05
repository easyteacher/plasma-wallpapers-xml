/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imageproxymodel.h"

#include <QFileInfo>

#include <KConfigGroup>
#include <KIO/OpenFileManagerWindowJob>
#include <KSharedConfig>

#include "imagelistmodel.h"
#include "packagelistmodel.h"
#include "xmlimagelistmodel.h"

ImageProxyModel::ImageProxyModel(const QStringList &customPaths, const QSize &targetSize, QObject *parent)
    : QConcatenateTablesProxyModel(parent)
    , m_imageModel(new ImageListModel(customPaths, targetSize, this))
    , m_packageModel(new PackageListModel(customPaths, targetSize, this))
    , m_xmlModel(new XmlImageListModel(customPaths, targetSize, this))
{
    connect(m_imageModel, &AbstractImageListModel::loaded, this, &ImageProxyModel::slotHandleLoaded);
    connect(m_packageModel, &AbstractImageListModel::loaded, this, &ImageProxyModel::slotHandleLoaded);
    connect(m_xmlModel, &AbstractImageListModel::loaded, this, &ImageProxyModel::slotHandleLoaded);
}

QHash< int, QByteArray > ImageProxyModel::roleNames() const
{
    const auto models = sourceModels();

    if (!models.empty()) {
        return models.constFirst()->roleNames();
    }

    return QConcatenateTablesProxyModel::roleNames();
}

int ImageProxyModel::count() const
{
    return rowCount();
}

int ImageProxyModel::indexOf(const QString &packagePath) const
{
    int idx = -1;

    const auto models = sourceModels();

    for (const auto &m : models) {
        idx = static_cast<const AbstractImageListModel *>(m)->indexOf(packagePath);

        if (idx >= 0) {
            return mapFromSource(m->index(idx, 0)).row();
        }
    }

    return idx;
}

void ImageProxyModel::reload()
{
    const auto models = sourceModels();

    if (models.empty()) {
        return;
    }

    for (const auto &m : models) {
        static_cast<AbstractImageListModel *>(m)->reload();
    }
}

void ImageProxyModel::addBackground(const QString &_path)
{
    QString path = _path;

    if (path.startsWith(QLatin1String("file://"))) {
        path.remove(0, 7);
    }

    const QFileInfo info(path);

    QStringList results;

    if (info.isDir()) {
        results = m_packageModel->addBackground(path);
    } else {
        if (info.suffix() == QLatin1String("xml")) {
            results = m_xmlModel->addBackground(path);
        } else {
            results = m_imageModel->addBackground(path);
        }
    }

    if (!results.empty()) {
        m_pendingAddition.append(results);
    }
}

void ImageProxyModel::removeBackground(const QString &path)
{
    const QFileInfo info(path);

    if (info.isDir()) {
        m_packageModel->removeBackground(path);
    } else if (info.isFile()) {
        m_imageModel->removeBackground(path);
    } else {
        const QUrl url(path);

        if (url.scheme() == QLatin1String("image") && url.host() == QLatin1String("gnome-wp-list")) {
            m_xmlModel->removeBackground(path);
        }
    }
}

void ImageProxyModel::commitAddition()
{
    if (m_pendingAddition.empty()) {
        return;
    }

    KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("plasmarc")), QStringLiteral("Wallpapers"));
    QStringList list = cfg.readEntry("usersWallpapers", QStringList{});

    list += m_pendingAddition;
    list.removeDuplicates();

    cfg.writeEntry("usersWallpapers", list);

    m_pendingAddition.clear();
}

void ImageProxyModel::commitDeletion()
{
    QStringList pendingList;

    for (int row = 0; row < rowCount(); row++) {
        QModelIndex idx = index(row, 0);

        if (idx.data(PendingDeletionRole).toBool()) {
            pendingList.append(idx.data(PackageNameRole).toString());
        }
    }

    for (const QString &p : std::as_const(pendingList)) {
        removeBackground(p);
    }

    // Update the config
    KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("plasmarc")), QStringLiteral("Wallpapers"));
    const QStringList list = cfg.readEntry("usersWallpapers", QStringList{});
    QStringList updatedList;

    std::copy_if(list.cbegin(), list.cend(), std::back_inserter(updatedList), [&pendingList](const QString &p) {
        return !pendingList.contains(p);
    });

    cfg.writeEntry("usersWallpapers", updatedList);
    cfg.sync();
}

void ImageProxyModel::openContainingFolder(int row) const
{
    KIO::highlightInFileManager({index(row, 0).data(PathRole).toUrl()});
}

void ImageProxyModel::slotHandleLoaded(AbstractImageListModel *model)
{
    disconnect(model, &AbstractImageListModel::loaded, this, 0);

    connect(model, &AbstractImageListModel::countChanged, this, &ImageProxyModel::countChanged);
    connect(this, &ImageProxyModel::targetSizeChanged, model, &AbstractImageListModel::slotTargetSizeChanged);

    addSourceModel(model);
}
