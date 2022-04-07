/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEPROXYMODEL_H
#define IMAGEPROXYMODEL_H

#include <QConcatenateTablesProxyModel>
#include <QSize>

#include "imageroles.h"

class AbstractImageListModel;
class ImageListModel;
class PackageListModel;
class XmlImageListModel;

/**
 * @todo write docs
 */
class ImageProxyModel : public QConcatenateTablesProxyModel, public ImageRoles
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit ImageProxyModel(const QStringList &customPaths, const QSize &targetSize, QObject *parent);

    QHash<int, QByteArray> roleNames() const override;

    int count() const;
    Q_INVOKABLE int indexOf(const QString &packagePath) const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void addBackground(const QString &_path);
    void removeBackground(const QString &path);

    Q_INVOKABLE void commitAddition();
    Q_INVOKABLE void commitDeletion();

    Q_INVOKABLE void openContainingFolder(int row) const;

Q_SIGNALS:
    void countChanged();
    void targetSizeChanged(const QSize &size);

private Q_SLOTS:
    void slotHandleLoaded(AbstractImageListModel *model);

private:
    ImageListModel *m_imageModel;
    PackageListModel *m_packageModel;
    XmlImageListModel *m_xmlModel;

    QStringList m_pendingAddition;
};

#endif // IMAGEPROXYMODEL_H
