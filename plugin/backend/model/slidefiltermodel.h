/*
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <random>

#include <QSortFilterProxyModel>
#include <QVector>
#include <QFileInfo>

#include "imageroles.h"
#include "sortingmode.h"

class SlideFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool usedInConfig MEMBER m_usedInConfig NOTIFY usedInConfigChanged);

public:
    explicit SlideFilterModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;

    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    void setSourceModel(QAbstractItemModel *sourceModel) override;

    void setSortingMode(SortingMode::Mode slideshowMode, bool slideshowFoldersFirst);
    void invalidate();

    Q_INVOKABLE int indexOf(const QString &packagePath) const;
    Q_INVOKABLE void openContainingFolder(int rowIndex) const;

public Q_SLOTS:
    void invalidateFilter();

Q_SIGNALS:
    void usedInConfigChanged();

private:
    void buildRandomOrder();

    QString getLocalFilePath(const QModelIndex& modelIndex) const;
    QString getFilePathWithDir(const QFileInfo& fileInfo) const;

    QVector<int> m_randomOrder;
    SortingMode::Mode m_SortingMode;
    bool m_SortingFoldersFirst;
    bool m_usedInConfig;
    std::mt19937 m_random;
};
