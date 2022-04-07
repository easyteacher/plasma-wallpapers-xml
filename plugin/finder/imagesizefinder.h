/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGESIZEFINDER_H
#define IMAGESIZEFINDER_H

#include <QObject>
#include <QRunnable>

/**
 * @todo write docs
 */
class ImageSizeFinder : public QObject,  public QRunnable
{
    Q_OBJECT

public:
    explicit ImageSizeFinder(const QString &path, QObject *parent = nullptr);

    void run() override;

Q_SIGNALS:
    void sizeFound(const QString &path, const QSize &size);

private:
    QString m_path;
};

#endif // IMAGESIZEFINDER_H
