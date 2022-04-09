/*
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "slidefiltermodel.h"

#include <algorithm>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QUrl>

#include "slidemodel.h"

SlideFilterModel::SlideFilterModel(QObject *parent)
    : QSortFilterProxyModel{parent}
    , m_SortingMode{SortingMode::Random}
    , m_SortingFoldersFirst{false}
    , m_usedInConfig{false}
    , m_random(std::random_device{}())
{
    srand(time(nullptr));
    setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
    connect(this, &SlideFilterModel::usedInConfigChanged, this, &SlideFilterModel::invalidateFilter);

    connect(this, &SlideFilterModel::rowsInserted, this, &SlideFilterModel::countChanged);
    connect(this, &SlideFilterModel::rowsRemoved, this, &SlideFilterModel::countChanged);
    connect(this, &SlideFilterModel::modelReset, this, &SlideFilterModel::countChanged);
}

QHash<int, QByteArray> SlideFilterModel::roleNames() const
{
    if (sourceModel()) {
        return sourceModel()->roleNames();
    }

    return QSortFilterProxyModel::roleNames();
}

bool SlideFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    auto index = sourceModel()->index(source_row, 0, source_parent);
    return m_usedInConfig || index.data(ImageRoles::ToggleRole).toBool();
}

void SlideFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (this->sourceModel()) {
        disconnect(this->sourceModel(), nullptr, this, nullptr);
    }

    QSortFilterProxyModel::setSourceModel(sourceModel);

    if (!m_usedInConfig && m_SortingMode == SortingMode::Random) {
        buildRandomOrder();
    }

    if (sourceModel) {
        connect(sourceModel, &QAbstractItemModel::modelReset, this, &SlideFilterModel::buildRandomOrder);
        connect(sourceModel, &QAbstractItemModel::rowsInserted, this, [this] {
            if (m_usedInConfig || m_SortingMode != SortingMode::Random) {
                return;
            }

            const int old_count = m_randomOrder.size();
            m_randomOrder.resize(this->sourceModel()->rowCount());
            std::iota(m_randomOrder.begin() + old_count, m_randomOrder.end(), old_count);
            std::shuffle(m_randomOrder.begin() + old_count, m_randomOrder.end(), m_random);

            // Update order
            QSortFilterProxyModel::invalidate();
        });
        connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, [this] {
            if (m_usedInConfig || m_SortingMode != SortingMode::Random) {
                return;
            }

            m_randomOrder.erase(std::remove_if(m_randomOrder.begin(),
                                               m_randomOrder.end(),
                                               [this](const int v) {
                                                   return v >= this->sourceModel()->rowCount();
                                               }),
                                m_randomOrder.end());

            QSortFilterProxyModel::invalidate();
        });
        connect(sourceModel, &QAbstractItemModel::dataChanged, this, &SlideFilterModel::dataChanged);
    }
}

int SlideFilterModel::count() const
{
    return rowCount();
}

bool SlideFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;

    switch (m_SortingMode) {
    case SortingMode::Random:
        if (m_usedInConfig) {
            return source_left.row() < source_right.row();
        }

        return m_randomOrder.indexOf(source_left.row()) < m_randomOrder.indexOf(source_right.row());

    case SortingMode::Alphabetical:
        if (m_SortingFoldersFirst) {
            QFileInfo leftFile(getLocalFilePath(source_left));
            QFileInfo rightFile(getLocalFilePath(source_right));
            QString leftFilePath = getFilePathWithDir(leftFile);
            QString rightFilePath = getFilePathWithDir(rightFile);

            if (leftFilePath == rightFilePath) {
                return QString::compare(leftFile.fileName(), rightFile.fileName(), cs) < 0;
            } else if (leftFilePath.startsWith(rightFilePath, cs)) {
                return true;
            } else if (rightFilePath.startsWith(leftFilePath, cs)) {
                return false;
            } else {
                return QString::compare(leftFilePath, rightFilePath, cs) < 0;
            }
        } else {
            QFileInfo leftFile(getLocalFilePath(source_left));
            QFileInfo rightFile(getLocalFilePath(source_right));

            return QString::compare(leftFile.fileName(), rightFile.fileName(), cs) < 0;
        }

    case SortingMode::AlphabeticalReversed:
        if (m_SortingFoldersFirst) {
            const QFileInfo leftFile(getLocalFilePath(source_left));
            const QFileInfo rightFile(getLocalFilePath(source_right));
            const QString leftFilePath = getFilePathWithDir(leftFile);
            const QString rightFilePath = getFilePathWithDir(rightFile);

            if (leftFilePath == rightFilePath) {
                return QString::compare(leftFile.fileName(), rightFile.fileName(), cs) > 0;
            } else if (leftFilePath.startsWith(rightFilePath, cs)) {
                return true;
            } else if (rightFilePath.startsWith(leftFilePath, cs)) {
                return false;
            } else {
                return QString::compare(leftFilePath, rightFilePath, cs) > 0;
            }
        } else {
            const QFileInfo leftFile(getLocalFilePath(source_left));
            const QFileInfo rightFile(getLocalFilePath(source_right));

            return QString::compare(leftFile.fileName(), rightFile.fileName(), cs) > 0;
        }

    case SortingMode::Modified: { // oldest first
        const QFileInfo leftFile(getLocalFilePath(source_left));
        const QFileInfo rightFile(getLocalFilePath(source_right));

        return leftFile.lastModified() < rightFile.lastModified();
    }

    case SortingMode::ModifiedReversed: { // newest first
        const QFileInfo leftFile(getLocalFilePath(source_left));
        const QFileInfo rightFile(getLocalFilePath(source_right));

        return !(leftFile.lastModified() < rightFile.lastModified());
    }
    }
    Q_UNREACHABLE();
}

void SlideFilterModel::setSortingMode(SortingMode::Mode slideshowMode, bool slideshowFoldersFirst)
{
    m_SortingMode = slideshowMode;
    m_SortingFoldersFirst = slideshowFoldersFirst;

    if (!m_usedInConfig && m_SortingMode == SortingMode::Random) {
        buildRandomOrder();
    }

    QSortFilterProxyModel::invalidate();
}

void SlideFilterModel::invalidate()
{
    if (!m_usedInConfig && m_SortingMode == SortingMode::Random) {
        std::shuffle(m_randomOrder.begin(), m_randomOrder.end(), m_random);
    }

    QSortFilterProxyModel::invalidate();
}

void SlideFilterModel::invalidateFilter()
{
    QSortFilterProxyModel::invalidateFilter();
}

int SlideFilterModel::indexOf(const QString &packagePath) const
{
    if (!sourceModel()) {
        return -1;
    }

    const int row = static_cast<SlideModel *>(sourceModel())->indexOf(packagePath);

    if (row < 0) {
        return -1;
    }

    return mapFromSource(sourceModel()->index(row, 0)).row();
}

void SlideFilterModel::openContainingFolder(int rowIndex) const
{
    auto sourceIndex = mapToSource(index(rowIndex, 0));
    static_cast<SlideModel *>(sourceModel())->openContainingFolder(sourceIndex.row());
}

void SlideFilterModel::buildRandomOrder()
{
    if (!sourceModel()) {
        return;
    }

    m_randomOrder.resize(sourceModel()->rowCount());
    std::iota(m_randomOrder.begin(), m_randomOrder.end(), 0);
    std::shuffle(m_randomOrder.begin(), m_randomOrder.end(), m_random);
}

QString SlideFilterModel::getLocalFilePath(const QModelIndex &modelIndex) const
{
    return modelIndex.data(ImageRoles::PathRole).toString();
}

QString SlideFilterModel::getFilePathWithDir(const QFileInfo &fileInfo) const
{
    return fileInfo.canonicalPath().append(QDir::separator());
}
