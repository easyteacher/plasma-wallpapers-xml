/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xmlslideshowupdatetimer.h"

#include "finder/xmlfinder.h"

XmlSlideshowUpdateTimer::XmlSlideshowUpdateTimer(QObject *parent)
    : QTimer(parent)
{
}

void XmlSlideshowUpdateTimer::adjustInterval(const QString &xmlpath)
{
    if (xmlpath.isEmpty()) {
        return;
    }

    const SlideshowData sData = XmlFinder::parseSlideshowXml(xmlpath, QSize(1920, 1080));

    m_intervals = slideshowTimeList(sData, m_totalTime);
    m_startTime = slideshowStartTime(sData);
}

void XmlSlideshowUpdateTimer::alignInterval()
{
    if (m_intervals.empty() || m_totalTime <= 0 || !m_startTime.isValid()) {
        return;
    }

    const qint64 timeDiff = m_startTime.secsTo(QDateTime::currentDateTime());

    // Align to remaining time
    const qint64 modTime = timeDiff % m_totalTime;
    int interval = 0;

    for (int i = 0; i < m_intervals.size(); i++) {
        const auto &p = m_intervals.at(i);

        if (p.second > modTime) {
            if (m_intervals.at(i - 1).first == 0) {
                // static, calculate remaining time
                interval = (p.second - modTime) * 1000; // sec to msec
                isTransition = false;
            } else {
                // transition
                interval = std::min<int>(p.second - modTime, 600) * 1000;
                isTransition = true;
            }

            break;
        }
    }

    // overflow
    if (interval < 0) {
        interval = std::numeric_limits<int>::max();
    }

    // At least 1min
    setInterval(std::max(60 * 1000, interval));
    start();
}

QDateTime XmlSlideshowUpdateTimer::slideshowStartTime(const SlideshowData &sData)
{
    QDateTime startTime = sData.starttime;

    if (startTime.isNull() || startTime > QDateTime::currentDateTime()) {
        // Use 0:00 as the start time
        startTime = QDate::currentDate().startOfDay().addDays(-1);

        if (sData.starttime.time().isValid()) {
            startTime.setTime(sData.starttime.time());
        }
    }

    return startTime;
}

QList<std::pair<int, qint64>> XmlSlideshowUpdateTimer::slideshowTimeList(const SlideshowData &sData, qint64 &totalTime)
{
    QList<std::pair<int, qint64>> timeList;
    totalTime = 0;

    for (const auto &item : std::as_const(sData.data)) {
        timeList.append(std::make_pair(item.dataType, totalTime));
        totalTime += item.duration;
    }

    timeList.append(std::make_pair(0, totalTime));

    return timeList;
}
