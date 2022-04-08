/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGELISTMODEL_H
#define IMAGELISTMODEL_H

#include "abstractimagelistmodel.h"

#include <QSet>

/**
 * @todo write docs
 */
class ImageListModel : public AbstractImageListModel
{
    Q_OBJECT

public:
    /**
     * Default constructor
     */
    explicit ImageListModel(const QStringList &customPaths, const QSize &targetSize, QObject *parent = nullptr);
    ~ImageListModel() override = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int indexOf(const QString &path) const override;

    void load(const QStringList &customPaths = {}) override;

public Q_SLOTS:
    QStringList addBackground(const QString &path) override;
    void removeBackground(const QString &path) override;

protected Q_SLOTS:
    void slotHandleImageFound(const QStringList &paths);

private:
    QStringList m_data;
};

#endif // IMAGELISTMODEL_H
