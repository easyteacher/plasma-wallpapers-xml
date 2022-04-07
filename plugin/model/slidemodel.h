/*
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QConcatenateTablesProxyModel>
#include <QSize>

#include "imageroles.h"

class ImageProxyModel;

/**
 * One folder = One source model
 */
class SlideModel : public QConcatenateTablesProxyModel, public ImageRoles
{
    Q_OBJECT

public:
    explicit SlideModel(const QSize &targetSize, QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int indexOf(const QString &packagePath) const;
    void openContainingFolder(int row) const;

    /**
     * @return added directories
     */
    QStringList addDirs(const QStringList &dirs);
    void removeDir(const QString &dir);
    void setSlidePaths(const QStringList &slidePaths);

    void setUncheckedSlides(const QStringList &uncheckedSlides);

Q_SIGNALS:
    void targetSizeChanged(const QSize &size);

private:
    QHash<QString, ImageProxyModel *> m_models;
    QHash<QString, bool> m_checkedTable;

    QSize m_targetSize;
};
