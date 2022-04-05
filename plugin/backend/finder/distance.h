/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CALCULATEDISTANCE_H
#define CALCULATEDISTANCE_H

#include <QSize>

/**
 * Compute difference of areas
 */
static float distance(const QSize &size, const QSize &desired)
{

    float desiredAspectRatio = (desired.height() > 0) ? desired.width() / static_cast<float>(desired.height()) : 0;
    float candidateAspectRatio = (size.height() > 0) ? size.width() / static_cast<float>(size.height()) : std::numeric_limits<float>::max();

    float delta = size.width() - desired.width();
    delta = delta >= 0.0 ? delta : -delta * 2; // Penalize for scaling up

    return std::abs(candidateAspectRatio - desiredAspectRatio) * 25000 + delta;
};

#endif
