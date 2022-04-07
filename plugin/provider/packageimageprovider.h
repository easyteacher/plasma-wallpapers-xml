/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PACKAGEIMAGEPROVIDER_H
#define PACKAGEIMAGEPROVIDER_H

#include <QQuickAsyncImageProvider>
#include <QThreadPool>

/**
 * @todo write docs
 */
class PackageImageProvider : public QQuickAsyncImageProvider
{
public:
    explicit PackageImageProvider();

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

private:
    QThreadPool m_pool;
};

#endif // PACKAGEIMAGEPROVIDER_H
