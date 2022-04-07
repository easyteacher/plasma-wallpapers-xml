/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xmlfinder.h"

#include <QCollator>
#include <QDir>
#include <QUrlQuery>
#include <QXmlStreamReader>

#include "distance.h"
#include "findsymlinktarget.h"

XmlFinder::XmlFinder(const QStringList& paths, const QSize &targetSize, QObject* parent)
    : QObject(parent)
    , m_paths(paths)
    , m_targetSize(targetSize)
{
}

void XmlFinder::run()
{
    QStringList xmls;

    QDir dir;
    dir.setFilter(QDir::AllDirs | QDir::Files | QDir::Readable);
    dir.setNameFilters({QStringLiteral("*.xml")});

    int i;
    for (i = 0; i < m_paths.size(); ++i) {
        const QString &path = m_paths.at(i);

        if (QUrl url(path); url.scheme() == QLatin1String("image") && url.host() == QLatin1String("gnome-wp-list")) {
            QUrlQuery urlQuery(url);
            const QString root = urlQuery.queryItemValue(QStringLiteral("_root"));

            if (root.endsWith(QLatin1String(".xml"))) {
                xmls.append(root);
            }

            continue;
        }

        if (QFileInfo info(path); path.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive) && info.isFile()) {
            xmls.append(findSymlinkTarget(info));
            continue;
        }

        dir.setPath(path);
        const QFileInfoList files = dir.entryInfoList();

        for (const QFileInfo &wp : files) {
            if (wp.isFile()) {
                xmls.append(findSymlinkTarget(wp));
            } else {
                const QString name = wp.fileName();

                if (name.startsWith('.')) {
                    continue;
                }

                // add this to the directories we should be looking at
                m_paths.append(wp.filePath());
            }
        }
    }

    xmls.removeDuplicates();

    QList<WallpaperItem> packages;

    for (const QString &path : std::as_const(xmls)) {
        packages += parseXml(path, m_targetSize);
    }

    sort(packages);

    Q_EMIT xmlFound(packages);
}

void XmlFinder::sort(QList<WallpaperItem> &list)
{
    QCollator collator;

    // Make sure 2 comes before 10
    collator.setNumericMode(true);
    // Behave like Dolphin with natural sorting enabled
    collator.setCaseSensitivity(Qt::CaseInsensitive);

    const auto compare = [&collator](const WallpaperItem &a, const WallpaperItem &b) {
        // Checking if less than zero makes ascending order (A-Z)
        return collator.compare(a.name, b.name) < 0;
    };

    std::stable_sort(list.begin(), list.end(), compare);
}

QList<WallpaperItem> XmlFinder::parseXml(const QString &path, const QSize &targetSize)
{
    QList<WallpaperItem> results;
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        return results;
    }

    QXmlStreamReader xml(&file);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QLatin1String("wallpaper")) {
            WallpaperItem item;

            while (!xml.atEnd()) {
                xml.readNext();

                if (xml.isEndElement()) {
                    if (xml.name() == QLatin1String("wallpaper")) {
                        if (item.name.isEmpty()) {
                            item.name = QFileInfo(item.filename).baseName();
                        }

                        item._root = path;
                        item.path = convertToUrl(item);

                        results.append(item);
                        break;
                    } else {
                        continue;
                    }
                }

                if (xml.name() == QLatin1String("name")) {
                    /* no pictures available for the specified parameters */
                    item.name = xml.readElementText();
                } else if (xml.name() == QLatin1String("filename")) {
                    item.filename = xml.readElementText();
                    if (item.filename.endsWith(QLatin1String(".xml"),Qt::CaseInsensitive)) {
                        item.slideshow = parseSlideshowXml(item.filename, targetSize);
                    }
                } else if (xml.name() == QLatin1String("filename-dark")) {
                    item.filename_dark = xml.readElementText();
                } else if (xml.name() == QLatin1String("author")) {
                    item.author = xml.readElementText();
                }
            }
        }
    }

    XmlFinder::sort(results);

    return results;
}

SlideshowData XmlFinder::parseSlideshowXml(const QString &path, const QSize &targetSize)
{
    SlideshowData data;
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        return data;
    }

    QXmlStreamReader xml(&file);
    QXmlStreamReader::TokenType token;

    while (!xml.atEnd()) {
        token = xml.readNext();

        if (token == QXmlStreamReader::Comment) {
            continue;
        }

        if (xml.isStartElement() && xml.name() == QLatin1String("background")) {
            while (!xml.atEnd()) { // background
                token = xml.readNext();

                if (token == QXmlStreamReader::Comment) {
                    continue;
                }

                if (xml.isEndElement()) {
                    if (xml.name() == QLatin1String("background")) {
                        break;
                    } else {
                        continue;
                    }
                }

                if (xml.isStartElement()) {
                    if (xml.name() == QLatin1String("starttime")) {
                        while (!xml.atEnd()) { // starttime
                            token = xml.readNext();

                            if (token == QXmlStreamReader::Comment) {
                                continue;
                            }

                            if (xml.isEndElement()) {
                                if (xml.name() == QLatin1String("starttime")) {
                                    break; // starttime
                                } else {
                                    continue;
                                }
                            }

                            if (xml.name() == QLatin1String("year")) {
                                data.starttime = data.starttime.addYears(xml.readElementText().toInt());
                            } else if (xml.name() == QLatin1String("month")) {
                                data.starttime = data.starttime.addMonths(xml.readElementText().toInt());
                            } else if (xml.name() == QLatin1String("day")) {
                                data.starttime = data.starttime.addDays(xml.readElementText().toInt());
                            } else if (xml.name() == QLatin1String("hour")) {
                                data.starttime = data.starttime.addSecs(xml.readElementText().toInt() * 3600);
                            } else if (xml.name() == QLatin1String("minute")) {
                                data.starttime = data.starttime.addSecs(xml.readElementText().toInt() * 60);
                            } else if (xml.name() == QLatin1String("second")) {
                                data.starttime = data.starttime.addSecs(xml.readElementText().toInt());
                            }
                        }
                    } else if (xml.name() == QLatin1String("static")) {
                        SlideshowItemData sdata;
                        sdata.dataType = 0;

                        while (!xml.atEnd()) {
                            token = xml.readNext();

                            if (token == QXmlStreamReader::Comment) {
                                continue;
                            }

                            if (xml.isEndElement()) {
                                if (xml.name() == QLatin1String("static")) {
                                    if (!sdata.file.isEmpty()) {
                                        data.data.append(sdata);
                                    }
                                    break;
                                } else {
                                    continue;
                                }
                            }

                            if (xml.name() == QLatin1String("duration")) {
                                sdata.duration = xml.readElementText().toDouble();
                            } else if (xml.name() == QLatin1String("file")) {
                                const QStringList results = xml.readElementText(QXmlStreamReader::IncludeChildElements).simplified().split(' ');

                                if (results.size() == 1) {
                                    sdata.file = results.constFirst();
                                } else {
                                    sdata.file = findPreferredImage(results, targetSize);
                                }
                            }
                        }
                    } else if (xml.name() == QLatin1String("transition")) {
                        SlideshowItemData tdata;
                        tdata.dataType = 1;

                        if (auto attr = xml.attributes(); attr.hasAttribute(QLatin1String("type"))) {
                            tdata.type = attr.value(QLatin1String("type")).toString();
                        }

                        while (!xml.atEnd()) { // static
                            token = xml.readNext();

                            if (token == QXmlStreamReader::Comment) {
                                continue;
                            }

                            if (xml.isEndElement()) {
                                if (xml.name() == QLatin1String("transition")) {
                                    if (!tdata.from.isEmpty() && !tdata.to.isEmpty()) {
                                        data.data.append(tdata);
                                    }
                                    break; // static
                                } else {
                                    continue;
                                }
                            }

                            if (xml.name() == QLatin1String("duration")) {
                                tdata.duration = xml.readElementText().toDouble();
                            } else if (xml.name() == QLatin1String("from")) {
                                tdata.from = xml.readElementText();
                            } else if (xml.name() == QLatin1String("to")) {
                                tdata.to = xml.readElementText();
                            }
                        }
                    }
                }
            }
        }

        if (xml.isEndElement() && xml.name() == QLatin1String("background")) {
            break;
        }
    }

    return data;
}

QUrl XmlFinder::convertToUrl(const WallpaperItem &item)
{
    QUrl url(QStringLiteral("image://gnome-wp-list/get"));

    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QStringLiteral("_root"), item._root);
    urlQuery.addQueryItem(QStringLiteral("filename"), item.filename);
    urlQuery.addQueryItem(QStringLiteral("filename_dark"), item.filename_dark);
    urlQuery.addQueryItem(QStringLiteral("name"), item.name);
    urlQuery.addQueryItem(QStringLiteral("author"), item.author);

    // Parse slideshow data if filename is an xml file, no need to save them in the url.

    url.setQuery(urlQuery);

    return url;
}

QString XmlFinder::findPreferredImage(const QStringList &pathList, const QSize &targetSize)
{
    if (pathList.empty()) {
        return QString();
    }

    QSize tSize = targetSize;

    if (tSize.isEmpty()) {
        tSize = QSize(1920, 1080);
    }

    QString preferred;

    float best = std::numeric_limits<float>::max();

    for (const auto &p : pathList) {
        QSize candidate = resSize(QFileInfo(p).baseName());

        if (candidate.isEmpty()) {
            continue;
        }

        float dist = distance(candidate, tSize);

        if (preferred.isEmpty() || dist < best) {
            preferred = p;
            best = dist;
        }
    }

    return preferred;
}
