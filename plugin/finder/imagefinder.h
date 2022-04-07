/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEFINDER_H
#define IMAGEFINDER_H

#include <QObject>
#include <QRunnable>

class QFileInfo;

QStringList suffixes();

/**
 * @todo write docs
 */
class ImageFinder : public QObject, public QRunnable
{
    Q_OBJECT

public:
    ImageFinder(const QStringList &paths, QObject *parent = nullptr);

    void run() override;

Q_SIGNALS:
    void imageFound(const QStringList &paths);

private:
    void sort(QStringList &list) const;

    QStringList m_paths;
};

#endif // IMAGEFINDER_H
