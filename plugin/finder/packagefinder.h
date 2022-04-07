/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PACKAGEFINDER_H
#define PACKAGEFINDER_H

#include <QObject>
#include <QRunnable>
#include <QSize>

#include <KPackage/Package>

void findPreferredImageInPackage(KPackage::Package &package, const QSize &targetSize);
QString packageDisplayName(const KPackage::Package &b);

/**
 * @todo write docs
 */
class PackageFinder : public QObject,  public QRunnable
{
    Q_OBJECT

public:
    PackageFinder(const QStringList &paths, const QSize &targetSize, QObject *parent = nullptr);

    void run() override;

Q_SIGNALS:
    void packageFound(const QList<KPackage::Package> &packages);

private:
    void sort(QList<KPackage::Package> &list) const;

    QStringList m_paths;
    QSize m_targetSize;
};

#endif // PACKAGEFINDER_H
