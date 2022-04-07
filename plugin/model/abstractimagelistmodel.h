/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTIMAGELISTMODEL_H
#define ABSTRACTIMAGELISTMODEL_H

#include <QAbstractListModel>
#include <QCache>
#include <QSize>
#include <QStandardPaths>

#include <KDirWatch>

#include "imageroles.h"

class QPixmap;
class KFileItem;

static const QString s_localImageDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/wallpapers/");

/**
 * @todo write docs
 */
class AbstractImageListModel : public QAbstractListModel, public ImageRoles
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit AbstractImageListModel(const QStringList &customPaths, const QSize &targetSize, QObject *parent = nullptr);
    ~AbstractImageListModel() override = default;

    QHash<int, QByteArray> roleNames() const override;

    int count() const;
    virtual int indexOf(const QString &path) const = 0;

    virtual void load(const QStringList &customPaths = {}) = 0;
    /**
     * Reload when target size changes or a new package is installed
     */
    void reload();

public Q_SLOTS:
    virtual QStringList addBackground(const QString &path) = 0;
    virtual void removeBackground(const QString &path) = 0;

    void slotTargetSizeChanged(const QSize &size);

Q_SIGNALS:
    void countChanged();
    void loaded(AbstractImageListModel *model);

protected:
    void asyncGetPreview(const QString &path, const QPersistentModelIndex &index) const;
    void asyncGetImageSize(const QString &path, const QPersistentModelIndex &index) const;

    bool m_loading = false;

    QSize m_screenshotSize;
    QSize m_targetSize;

    QCache<QString, QPixmap> m_imageCache;
    QCache<QString, QSize> m_imageSizeCache;

    mutable QHash<QString, QPersistentModelIndex> m_previewJobsUrls;

    QHash<QString, bool> m_pendingDeletion;
    QStringList m_removableWallpapers;
    QStringList m_customPaths;

    KDirWatch m_dirWatch;

private Q_SLOTS:
    void slotHandleImageSizeFound(const QString &path, const QSize &size);
    void slotHandlePreview(const KFileItem &item, const QPixmap &preview);
    void slotHandlePreviewFailed(const KFileItem &item);

private:
    mutable QHash<QString, QPersistentModelIndex> m_sizeJobsUrls;
};

#endif // ABSTRACTIMAGELISTMODEL_H
