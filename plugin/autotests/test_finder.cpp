/*
    SPDX-FileCopyrightText: 2016 Antonio Larrosa <larrosa@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDebug>
#include <QtTest>

#include <KPackage/PackageLoader>

#include "../finder/imagefinder.h"
#include "../finder/packagefinder.h"
#include "../finder/xmlfinder.h"
#include "constant.h"

class FinderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testImageFinder();

    void testFindPreferredSizeInPackage_data();
    void testFindPreferredSizeInPackage();
    void testPackageFinder();

    void testFindPreferredSizeInXml_data();
    void testFindPreferredSizeInXml();
    void testXmlFinder();

private:
    QStringList m_paths;
    QDir m_dataDir;
};

void FinderTest::initTestCase()
{
    m_dataDir = QDir(QFINDTESTDATA("testdata/default"));
    QVERIFY(!m_dataDir.isEmpty());
}

void FinderTest::testImageFinder()
{
    ImageFinder *finder = new ImageFinder({m_dataDir.absolutePath()});
    QSignalSpy spy(finder, &ImageFinder::imageFound);

    QThreadPool::globalInstance()->start(finder);

    spy.wait(10 * 1000);

    QCOMPARE(spy.count(), 1);

    const auto items = spy.takeFirst().constFirst().toStringList();
    // Total 2 images in the directory, but one is symlink and should not be added to the list.
    QCOMPARE(items.size(), 1);
    QCOMPARE(items.constFirst(), m_dataDir.absoluteFilePath(QStringLiteral("wallpaper.jpg")));
}

void FinderTest::testFindPreferredSizeInPackage_data()
{
    // The list of possible screen resolutions to test and the appropriate images that should be chosen
    QTest::addColumn<QSize>("resolution");
    QTest::addColumn<QString>("expected");
    QTest::newRow("1280x1024") << QSize(1280, 1024) << QStringLiteral("1280x1024");
    QTest::newRow("1350x1080") << QSize(1350, 1080) << QStringLiteral("1280x1024");
    QTest::newRow("1440x1080") << QSize(1440, 1080) << QStringLiteral("1600x1200");
    QTest::newRow("1600x1200") << QSize(1600, 1200) << QStringLiteral("1600x1200");
    QTest::newRow("1920x1080") << QSize(1920, 1080) << QStringLiteral("1920x1080");
    QTest::newRow("1920x1200") << QSize(1920, 1200) << QStringLiteral("1920x1200");
    QTest::newRow("3840x2400") << QSize(3840, 2400) << QStringLiteral("3200x2000");
    QTest::newRow("4096x2160") << QSize(4096, 2160) << QStringLiteral("3840x2160");
    QTest::newRow("3840x2160") << QSize(3840, 2160) << QStringLiteral("3840x2160");
    QTest::newRow("3200x1800") << QSize(3200, 1800) << QStringLiteral("3200x1800");
    QTest::newRow("2048x1080") << QSize(2048, 1080) << QStringLiteral("1920x1080");
    QTest::newRow("1680x1050") << QSize(1680, 1050) << QStringLiteral("1680x1050");
    QTest::newRow("1400x1050") << QSize(1400, 1050) << QStringLiteral("1600x1200");
    QTest::newRow("1440x900") << QSize(1440, 900) << QStringLiteral("1440x900");
    QTest::newRow("1280x960") << QSize(1280, 960) << QStringLiteral("1600x1200");
    QTest::newRow("1280x854") << QSize(1280, 854) << QStringLiteral("1280x800");
    QTest::newRow("1280x800") << QSize(1280, 800) << QStringLiteral("1280x800");
    QTest::newRow("1280x720") << QSize(1280, 720) << QStringLiteral("1366x768");
    QTest::newRow("1152x768") << QSize(1152, 768) << QStringLiteral("1280x800");
    QTest::newRow("1024x768") << QSize(1024, 768) << QStringLiteral("1024x768");
    QTest::newRow("800x600") << QSize(800, 600) << QStringLiteral("1024x768");
    QTest::newRow("848x480") << QSize(848, 480) << QStringLiteral("1366x768");
    QTest::newRow("720x480") << QSize(720, 480) << QStringLiteral("1280x800");
    QTest::newRow("640x480") << QSize(640, 480) << QStringLiteral("1024x768");
    QTest::newRow("1366x768") << QSize(1366, 768) << QStringLiteral("1366x768");
    QTest::newRow("1600x814") << QSize(1600, 814) << QStringLiteral("1920x1080");
}

void FinderTest::testFindPreferredSizeInPackage()
{
    QFETCH(QSize, resolution);
    QFETCH(QString, expected);

    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Wallpaper/Images"));
    package.setPath(m_dataDir.absoluteFilePath(QStringLiteral("package")));

    QVERIFY(package.isValid());
    QVERIFY(package.metadata().isValid());

    PackageFinder::findPreferredImageInPackage(package, resolution);

    QVERIFY(package.filePath("preferred").contains(expected));
}

void FinderTest::testPackageFinder()
{
    PackageFinder *finder = new PackageFinder({m_dataDir.absolutePath()}, QSize(1920, 1080));
    QSignalSpy spy(finder, &PackageFinder::packageFound);

    QThreadPool::globalInstance()->start(finder);

    spy.wait(10 * 1000);

    QCOMPARE(spy.count(), 1);

    const auto items = spy.takeFirst().constFirst().value<QList<KPackage::Package>>();
    // Total 2 packages in the directory, but one package is broken and should not be added to the list.
    QCOMPARE(items.size(), 1);
    QCOMPARE(items.constFirst().filePath("preferred"), m_dataDir.absoluteFilePath(QStringLiteral("package/contents/images/1920x1080.jpg")));
}

void FinderTest::testFindPreferredSizeInXml_data()
{
    m_paths = QStringList{
        "/usr/share/wallpapers/dummy/contents/images/1280x1024.jpg"
        "/usr/share/wallpapers/dummy/contents/images/1350x1080.jpg",
        "/usr/share/wallpapers/dummy/contents/images/1440x1080.jpg",
        "/usr/share/wallpapers/dummy/contents/images/1600x1200.jpg",
        "/usr/share/wallpapers/dummy/contents/images/1920x1080.jpg",
        "/usr/share/wallpapers/dummy/contents/images/1920x1200.jpg",
        "/usr/share/wallpapers/dummy/contents/images/3840x2400.jpg",
    };

    // The list of possible screen resolutions to test and the appropriate images that should be chosen
    QTest::addColumn<QSize>("resolution");
    QTest::addColumn<QString>("expected");
    QTest::newRow("1280x1024") << QSize(1280, 1024) << QStringLiteral("1280x1024");
    QTest::newRow("1350x1080") << QSize(1350, 1080) << QStringLiteral("1350x1080");
    QTest::newRow("1440x1080") << QSize(1440, 1080) << QStringLiteral("1440x1080");
    QTest::newRow("1600x1200") << QSize(1600, 1200) << QStringLiteral("1600x1200");
    QTest::newRow("1920x1080") << QSize(1920, 1080) << QStringLiteral("1920x1080");
    QTest::newRow("1920x1200") << QSize(1920, 1200) << QStringLiteral("1920x1200");
    QTest::newRow("3840x2400") << QSize(3840, 2400) << QStringLiteral("3840x2400");
    QTest::newRow("4096x2160") << QSize(4096, 2160) << QStringLiteral("1920x1080");
    QTest::newRow("3840x2160") << QSize(3840, 2160) << QStringLiteral("1920x1080");
    QTest::newRow("3200x1800") << QSize(3200, 1800) << QStringLiteral("1920x1080");
    QTest::newRow("2048x1080") << QSize(2048, 1080) << QStringLiteral("1920x1080");
    QTest::newRow("1680x1050") << QSize(1680, 1050) << QStringLiteral("1920x1200");
    QTest::newRow("1400x1050") << QSize(1400, 1050) << QStringLiteral("1440x1080");
    QTest::newRow("1440x900") << QSize(1440, 900) << QStringLiteral("1920x1200");
    QTest::newRow("1280x960") << QSize(1280, 960) << QStringLiteral("1440x1080");
    QTest::newRow("1280x854") << QSize(1280, 854) << QStringLiteral("1920x1200");
    QTest::newRow("1280x800") << QSize(1280, 800) << QStringLiteral("1920x1200");
    QTest::newRow("1280x720") << QSize(1280, 720) << QStringLiteral("1920x1080");
    QTest::newRow("1152x768") << QSize(1152, 768) << QStringLiteral("1920x1200");
    QTest::newRow("1024x768") << QSize(1024, 768) << QStringLiteral("1440x1080");
    QTest::newRow("800x600") << QSize(800, 600) << QStringLiteral("1440x1080");
    QTest::newRow("848x480") << QSize(848, 480) << QStringLiteral("1920x1080");
    QTest::newRow("720x480") << QSize(720, 480) << QStringLiteral("1920x1200");
    QTest::newRow("640x480") << QSize(640, 480) << QStringLiteral("1440x1080");
    QTest::newRow("1366x768") << QSize(1366, 768) << QStringLiteral("1920x1080");
    QTest::newRow("1600x814") << QSize(1600, 814) << QStringLiteral("1920x1080");
}

void FinderTest::testFindPreferredSizeInXml()
{
    QFETCH(QSize, resolution);
    QFETCH(QString, expected);

    QVERIFY(XmlFinder::findPreferredImage(m_paths, resolution).contains(expected));
}

void FinderTest::testXmlFinder()
{
    XmlFinder *finder = new XmlFinder({m_dataDir.absolutePath()}, QSize(1920, 1080));
    QSignalSpy spy(finder, &XmlFinder::xmlFound);

    QThreadPool::globalInstance()->start(finder);

    spy.wait(10 * 1000);

    QCOMPARE(spy.count(), 1);

    const auto items = spy.takeFirst().constFirst().value<QList<WallpaperItem>>();
    QCOMPARE(items.size(), 2);

    // Light/Dark
    const auto &lditem = items.at(0);

    QCOMPARE(lditem.name, xmlImageName1);
    QCOMPARE(lditem.filename, m_dataDir.absoluteFilePath("xml/.light.jpg"));
    QCOMPARE(lditem.filename_dark, m_dataDir.absoluteFilePath("xml/.dark.jpg"));
    QCOMPARE(lditem._root, m_dataDir.absoluteFilePath("xml/lightdark.xml"));

    // Parse image://gnome-wp-list/get?...
    const QUrlQuery urlQuery(lditem.path);
    const QString filename = urlQuery.queryItemValue(QStringLiteral("filename"));
    const QString filename_dark = urlQuery.queryItemValue(QStringLiteral("filename_dark"));
    QCOMPARE(filename, m_dataDir.absoluteFilePath("xml/.light.jpg"));
    QCOMPARE(filename_dark, m_dataDir.absoluteFilePath("xml/.dark.jpg"));

    // Slideshow
    const auto &sItem = items.at(1);

    QCOMPARE(sItem.name, xmlImageName2);
    QCOMPARE(sItem.filename, m_dataDir.absoluteFilePath("xml/timeofday.xml"));
    QCOMPARE(sItem._root, m_dataDir.absoluteFilePath("xml/lightdark.xml"));
    QCOMPARE(sItem.slideshow.starttime, QDateTime(QDate(2022, 5, 24), QTime(8, 0, 0)));

    // Static/Transition data
    QCOMPARE(sItem.slideshow.data.size(), 4);
    // Static 1
    QCOMPARE(sItem.slideshow.data.at(0).file, m_dataDir.absoluteFilePath("xml/.light.jpg"));
    QCOMPARE(sItem.slideshow.data.at(0).duration, 36000);
    // Transition 1
    QCOMPARE(sItem.slideshow.data.at(1).from, m_dataDir.absoluteFilePath("xml/.light.jpg"));
    QCOMPARE(sItem.slideshow.data.at(1).to, m_dataDir.absoluteFilePath("xml/.dark.jpg"));
    QCOMPARE(sItem.slideshow.data.at(1).duration, 7200);
    // Static 2
    QCOMPARE(sItem.slideshow.data.at(2).file, m_dataDir.absoluteFilePath("xml/.dark.jpg"));
    QCOMPARE(sItem.slideshow.data.at(2).duration, 36000);
    // Transition 2
    QCOMPARE(sItem.slideshow.data.at(3).from, m_dataDir.absoluteFilePath("xml/.dark.jpg"));
    QCOMPARE(sItem.slideshow.data.at(3).to, m_dataDir.absoluteFilePath("xml/.light.jpg"));
    QCOMPARE(sItem.slideshow.data.at(3).duration, 7200);
}

QTEST_GUILESS_MAIN(FinderTest)

#include "test_finder.moc"
