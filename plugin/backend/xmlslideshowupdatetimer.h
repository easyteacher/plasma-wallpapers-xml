/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SLIDESHOWUPDATETIMER_H
#define SLIDESHOWUPDATETIMER_H

#include <QDateTime>
#include <QTimer>

/**
 * @todo write docs
 */
class XmlSlideshowUpdateTimer : public QTimer
{
    Q_OBJECT

public:
    XmlSlideshowUpdateTimer(QObject *parent = nullptr);

    void adjustInterval(const QString &xmlpath);

    bool isTransition = false;

public Q_SLOTS:
    void alignInterval();

Q_SIGNALS:
    void isTransitionChanged();

private:
    QList<std::pair<int /* type */, qint64>> m_intervals;
    qint64 m_totalTime = -1;
    QDateTime m_startTime;
};

#endif // SLIDESHOWUPDATETIMER_H
