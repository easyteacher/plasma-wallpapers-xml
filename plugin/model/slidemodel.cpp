/*
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "slidemodel.h"

#include <QFileInfo>

#include <KIO/OpenFileManagerWindowJob>

#include "abstractimagelistmodel.h"
#include "imageproxymodel.h"

SlideModel::SlideModel(const QSize &targetSize, QObject *parent)
    : QConcatenateTablesProxyModel(parent)
    , m_targetSize(targetSize)
{
    connect(this, &SlideModel::targetSizeChanged, [this](const QSize &s) { m_targetSize = s; });
}

QHash<int, QByteArray> SlideModel::roleNames() const
{
    const auto models = sourceModels();

    if (!models.empty()) {
        return models.constFirst()->roleNames();
    }

    return QConcatenateTablesProxyModel::roleNames();
}

QVariant SlideModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == ToggleRole) {
        return m_checkedTable.value(index.data(PackageNameRole).toString(), true);
    }

    return QConcatenateTablesProxyModel::data(index, role);
}

bool SlideModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    if (role == ToggleRole) {
        m_checkedTable[index.data(PackageNameRole).toString()] = value.toBool();

        Q_EMIT dataChanged(index, index, {ToggleRole});
        return true;
    }

    return QConcatenateTablesProxyModel::setData(index, value, role);
}

int SlideModel::indexOf(const QString &packagePath) const
{
    int idx = -1;

    for (const auto &m : m_models) {
        idx = m->indexOf(packagePath);

        if (idx >= 0) {
            return mapFromSource(m->index(idx, 0)).row();
        }
    }

    return idx;
}

void SlideModel::openContainingFolder(int row) const
{
    KIO::highlightInFileManager({index(row, 0).data(PathRole).toUrl()});
}

QStringList SlideModel::addDirs(const QStringList &dirs)
{
    QStringList added;

    for (const QString &p : dirs) {
        if (!m_models.contains(p) && QFileInfo(p).isDir()) {
            auto *m = new ImageProxyModel({p}, m_targetSize, this);

            connect(this, &SlideModel::targetSizeChanged, m, &ImageProxyModel::targetSizeChanged);

            m_models.insert(p, m);
            addSourceModel(m);
            added.append(p);
        }
    }

    return added;
}

void SlideModel::removeDir(const QString &dir)
{
    if (!m_models.contains(dir)) {
        return;
    }

    auto *m = m_models.take(dir);

    removeSourceModel(m);

    m->deleteLater();
}

void SlideModel::setSlidePaths(const QStringList &slidePaths)
{
    for (const auto &m : std::as_const(m_models)) {
        removeSourceModel(m);
        m->deleteLater();
    }

    m_models.clear();

    addDirs(slidePaths);
}

void SlideModel::setUncheckedSlides(const QStringList &uncheckedSlides)
{
    m_checkedTable.clear();

    for (const QString &p : uncheckedSlides) {
        m_checkedTable[p] = false;
    }
}
