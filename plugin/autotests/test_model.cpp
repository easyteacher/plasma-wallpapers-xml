/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <QDebug>
#include <QtTest>

#include "../model/imagelistmodel.h"
#include "../model/imageproxymodel.h"
#include "../model/packagelistmodel.h"
#include "../model/slidefiltermodel.h"
#include "../model/slidemodel.h"
#include "../model/xmlimagelistmodel.h"

class ModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testImageListModel();
    void testPackageListModel();
    void testXmlImageListModel();

    void testImageProxyModelLoadReload();
    void testImageProxyModelAddRemoveBackground();

    void testSlideModel();
    void testSlideFilterModel();

private:
    QDir m_dataDir;
    QDir m_alternateDir;
    QSize m_targetSize;
};

void ModelTest::initTestCase()
{
    m_dataDir = QDir(QFINDTESTDATA("testdata/default"));
    m_alternateDir = QDir(QFINDTESTDATA("testdata/alternate"));
    QVERIFY(!m_dataDir.isEmpty());
    QVERIFY(!m_alternateDir.isEmpty());

    m_targetSize = QSize(1920, 1080);
}

void ModelTest::testImageListModel()
{
    const auto model = new ImageListModel({m_dataDir.absolutePath()}, m_targetSize, this);
    QSignalSpy spy(model, &ImageListModel::countChanged);
    QSignalSpy dataChangedSpy(model, &ImageListModel::dataChanged);

    spy.wait(10 * 1000);
    QCOMPARE(spy.size(), 1);
    spy.clear();
    QCOMPARE(model->rowCount(), 1);

    // Test data
    QPersistentModelIndex idx = model->index(0, 0);
    QVERIFY(idx.isValid());

    QCOMPARE(idx.data(Qt::DisplayRole).toString(), QStringLiteral("wallpaper.jpg"));

    QCOMPARE(idx.data(ImageRoles::ScreenshotRole), QVariant()); // Not cahced yet
    dataChangedSpy.wait();
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::ScreenshotRole);
    QVERIFY(!idx.data(ImageRoles::ScreenshotRole).value<QPixmap>().isNull());

    QCOMPARE(idx.data(ImageRoles::AuthorRole).toString(), QString());

    QCOMPARE(idx.data(ImageRoles::ResolutionRole).toString(), QString());
    dataChangedSpy.wait();
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::ResolutionRole);
    QCOMPARE(idx.data(ImageRoles::ResolutionRole).toString(), QStringLiteral("1x1"));

    QCOMPARE(idx.data(ImageRoles::PathRole).toUrl(), QUrl::fromLocalFile(m_dataDir.absoluteFilePath(QStringLiteral("wallpaper.jpg"))));
    QCOMPARE(idx.data(ImageRoles::PackageNameRole).toString(), m_dataDir.absoluteFilePath(QStringLiteral("wallpaper.jpg")));

    QCOMPARE(idx.data(ImageRoles::RemovableRole).toBool(), false);
    QCOMPARE(idx.data(ImageRoles::PendingDeletionRole).toBool(), false);

    // Test addBackground and RemovableRole
    model->addBackground(m_alternateDir.absoluteFilePath(QStringLiteral("dummy.jpg")));
    QCOMPARE(spy.size(), 1);
    spy.clear();

    idx = model->index(0, 0); // This is the newly added item.
    QVERIFY(idx.isValid());

    QCOMPARE(idx.data(Qt::DisplayRole).toString(), QStringLiteral("dummy.jpg"));
    QCOMPARE(idx.data(ImageRoles::RemovableRole).toBool(), true);

    // Test PendingDeletionRole
    model->setData(idx, true, ImageRoles::PendingDeletionRole);
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::PendingDeletionRole);
    QCOMPARE(idx.data(ImageRoles::PendingDeletionRole).toBool(), true);

    model->deleteLater();
}

void ModelTest::testPackageListModel()
{
    const auto model = new PackageListModel({m_dataDir.absolutePath()}, m_targetSize, this);
    QSignalSpy spy(model, &PackageListModel::countChanged);
    QSignalSpy dataChangedSpy(model, &PackageListModel::dataChanged);

    spy.wait(10 * 1000);
    QCOMPARE(spy.size(), 1);
    spy.clear();
    QCOMPARE(model->rowCount(), 1);

    // Test data
    QPersistentModelIndex idx = model->index(0, 0);
    QVERIFY(idx.isValid());

    QCOMPARE(idx.data(Qt::DisplayRole).toString(), QStringLiteral("Honeywave (For test purpose, don't translate!)"));

    QCOMPARE(idx.data(ImageRoles::ScreenshotRole), QVariant()); // Not cahced yet
    dataChangedSpy.wait();
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::ScreenshotRole);
    QVERIFY(!idx.data(ImageRoles::ScreenshotRole).value<QPixmap>().isNull());

    QCOMPARE(idx.data(ImageRoles::AuthorRole).toString(), QStringLiteral("Ken Vermette"));

    QCOMPARE(idx.data(ImageRoles::ResolutionRole).toString(), QString());
    dataChangedSpy.wait();
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::ResolutionRole);
    QCOMPARE(idx.data(ImageRoles::ResolutionRole).toString(), QStringLiteral("1920x1080"));

    QCOMPARE(idx.data(ImageRoles::PathRole).toUrl(), QUrl::fromLocalFile(m_dataDir.absoluteFilePath(QStringLiteral("package/contents/images/1920x1080.jpg"))));
    QCOMPARE(idx.data(ImageRoles::PackageNameRole).toString(), m_dataDir.absoluteFilePath(QStringLiteral("package/")));

    QCOMPARE(idx.data(ImageRoles::RemovableRole).toBool(), false);
    QCOMPARE(idx.data(ImageRoles::PendingDeletionRole).toBool(), false);

    // Test addBackground and RemovableRole
    model->addBackground(m_alternateDir.absoluteFilePath(QStringLiteral("openSUSEdefault")));
    QCOMPARE(spy.size(), 1);
    spy.clear();

    idx = model->index(0, 0); // This is the newly added item.
    QVERIFY(idx.isValid());

    QCOMPARE(idx.data(Qt::DisplayRole).toString(), QStringLiteral("openSUSE default (For test purpose, don't translate!)"));
    QCOMPARE(idx.data(ImageRoles::RemovableRole).toBool(), true);

    // Test PendingDeletionRole
    model->setData(idx, true, ImageRoles::PendingDeletionRole);
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::PendingDeletionRole);
    QCOMPARE(idx.data(ImageRoles::PendingDeletionRole).toBool(), true);

    model->deleteLater();
}

void ModelTest::testXmlImageListModel()
{
    const auto model = new XmlImageListModel({m_dataDir.absolutePath()}, m_targetSize, this);
    QSignalSpy spy(model, &XmlImageListModel::countChanged);
    QSignalSpy dataChangedSpy(model, &XmlImageListModel::dataChanged);

    spy.wait(10 * 1000);
    QCOMPARE(spy.size(), 1);
    spy.clear();
    QCOMPARE(model->rowCount(), 2);

    // Test data
    QPersistentModelIndex idx = model->index(0, 0);
    QPersistentModelIndex idx2 = model->index(1, 0);
    QVERIFY(idx.isValid());

    QCOMPARE(idx.data(Qt::DisplayRole).toString(), QStringLiteral("Default Background (For test purpose, don't translate!)"));
    QCOMPARE(idx2.data(Qt::DisplayRole).toString(), QStringLiteral("Time of Day (For test purpose, don't translate!)"));

    // Test XmlPreviewGenerator
    QCOMPARE(idx.data(ImageRoles::ScreenshotRole), QVariant()); // Not cahced yet
    dataChangedSpy.wait();
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::ScreenshotRole);
    QVERIFY(!idx.data(ImageRoles::ScreenshotRole).value<QPixmap>().isNull());

    QCOMPARE(idx.data(ImageRoles::AuthorRole).toString(), QStringLiteral("KDE Contributor"));
    QCOMPARE(idx2.data(ImageRoles::AuthorRole).toString(), QString());

    QCOMPARE(idx.data(ImageRoles::ResolutionRole).toString(), QString());
    dataChangedSpy.wait();
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::ResolutionRole);
    // They have the same path.
    QCOMPARE(idx.data(ImageRoles::ResolutionRole).toString(), QStringLiteral("1x1"));
    QCOMPARE(idx2.data(ImageRoles::ResolutionRole).toString(), QStringLiteral("1x1"));

    QCOMPARE(idx.data(ImageRoles::PathRole).toUrl(), QUrl::fromLocalFile(m_dataDir.absoluteFilePath(QStringLiteral("xml/.light.jpg"))));

    QUrl packageUrl(idx.data(ImageRoles::PackageNameRole).toString());
    const QUrlQuery urlQuery(packageUrl);
    QCOMPARE(packageUrl.scheme(), QStringLiteral("image"));
    QCOMPARE(packageUrl.host(), QStringLiteral("gnome-wp-list"));
    QCOMPARE(urlQuery.queryItemValue(QStringLiteral("filename")), m_dataDir.absoluteFilePath("xml/.light.jpg"));
    QCOMPARE(urlQuery.queryItemValue(QStringLiteral("filename_dark")), m_dataDir.absoluteFilePath("xml/.dark.jpg"));

    QCOMPARE(idx.data(ImageRoles::RemovableRole).toBool(), false);
    QCOMPARE(idx.data(ImageRoles::PendingDeletionRole).toBool(), false);

    // Test addBackground and RemovableRole
    model->addBackground(m_alternateDir.absoluteFilePath(QStringLiteral("dummy.xml")));
    QCOMPARE(spy.size(), 1);
    spy.clear();

    idx = model->index(0, 0); // This is the newly added item.
    QVERIFY(idx.isValid());

    QCOMPARE(idx.data(Qt::DisplayRole).toString(), QStringLiteral("Dear (For test purpose, don't translate!)"));
    QCOMPARE(idx.data(ImageRoles::RemovableRole).toBool(), true);

    // Test PendingDeletionRole
    model->setData(idx, true, ImageRoles::PendingDeletionRole);
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::PendingDeletionRole);
    QCOMPARE(idx.data(ImageRoles::PendingDeletionRole).toBool(), true);

    model->deleteLater();
}

void ModelTest::testImageProxyModelLoadReload()
{
    const auto model = new ImageProxyModel({m_dataDir.absolutePath()}, m_targetSize, this);
    QSignalSpy spy(model, &ImageProxyModel::countChanged);

    const auto verifyModel = [&] {
        // Total 3 models
        spy.wait(10 * 1000);
        spy.wait(10 * 1000);
        spy.wait(10 * 1000);
        QCOMPARE(spy.size(), 3);
        spy.clear(); // Will compare again when testing reload()

        QCOMPARE(model->m_imageModel->count(), 1);
        QCOMPARE(model->m_packageModel->count(), 1);
        QCOMPARE(model->m_xmlModel->count(), 2);
        QCOMPARE(model->count(), 4);

        // Test indexOf()
        QVERIFY(model->indexOf(m_dataDir.absoluteFilePath(QStringLiteral("wallpaper.jpg"))) >= 0);

        QVERIFY(model->indexOf(m_dataDir.absoluteFilePath(QStringLiteral("package/"))) >= 0);
        QVERIFY(model->indexOf(m_dataDir.absoluteFilePath(QStringLiteral("brokenpackage/"))) < 0);

        const QString xmlPackageName = model->m_xmlModel->index(0, 0).data(ImageRoles::PackageNameRole).toString();
        QVERIFY(xmlPackageName.startsWith(QStringLiteral("image://gnome-wp-list/get?")));
        QVERIFY(model->indexOf(xmlPackageName) >= 0);
    };

    verifyModel();

    // Test reload()
    model->reload();
    verifyModel();

    model->deleteLater();
}

void ModelTest::testImageProxyModelAddRemoveBackground()
{
    const auto model = new ImageProxyModel({m_dataDir.absolutePath()}, m_targetSize, this);
    QSignalSpy spy(model, &ImageProxyModel::countChanged);

    spy.wait(10 * 1000);
    spy.wait(10 * 1000);
    spy.wait(10 * 1000);
    QCOMPARE(spy.size(), 3);
    spy.clear(); // Loaded

    int count = model->count();

    /*
     * Test addBackground()
     */
    // Case 1: add an existing wallpaper
    auto results = model->addBackground(m_dataDir.absoluteFilePath(QStringLiteral("wallpaper.jpg")));
    QCOMPARE(spy.size(), 0);
    QCOMPARE(results.size(), 0);

    // Case 2: add new wallpaper
    results = model->addBackground(m_alternateDir.absoluteFilePath(QStringLiteral("dummy.jpg")));
    QCOMPARE(spy.size(), 1);
    spy.clear();
    QCOMPARE(results.size(), 1);
    QCOMPARE(model->count(), ++count);

    results = model->addBackground(m_alternateDir.absoluteFilePath(QStringLiteral("dummy.xml")));
    QCOMPARE(spy.size(), 1);
    spy.clear();
    QCOMPARE(results.size(), 1);
    QCOMPARE(model->count(), ++count);

    const QString xmlPath = results.at(0);
    QVERIFY(xmlPath.startsWith(QStringLiteral("image://gnome-wp-list/get?")));

    results = model->addBackground(m_alternateDir.absoluteFilePath(QStringLiteral("openSUSEdefault")));
    QCOMPARE(spy.size(), 1);
    spy.clear();
    QCOMPARE(results.size(), 1);
    QCOMPARE(model->count(), ++count);

    QCOMPARE(model->m_pendingAddition.size(), 3);

    /*
     * Test removeBackground()
     */
    // Case 1: remove existing wallpapers
    model->removeBackground(m_alternateDir.absoluteFilePath(QStringLiteral("dummy.jpg")));
    QCOMPARE(spy.size(), 1);
    spy.clear();
    QCOMPARE(model->count(), --count);

    model->removeBackground(xmlPath);
    QCOMPARE(spy.size(), 1);
    spy.clear();
    QCOMPARE(model->count(), --count);

    model->removeBackground(m_alternateDir.absoluteFilePath(QStringLiteral("openSUSEdefault")));
    QCOMPARE(spy.size(), 1);
    spy.clear();
    QCOMPARE(model->count(), --count);

    // Case 2: remove an unexisting wallpaper
    model->removeBackground(xmlPath);
    QCOMPARE(spy.size(), 0);
    QCOMPARE(model->count(), count);

    QCOMPARE(model->m_pendingAddition.size(), 0);

    model->deleteLater();
}

void ModelTest::testSlideModel()
{
    const auto model = new SlideModel(m_targetSize, this);
    QSignalSpy spy(model, &SlideModel::rowsInserted);
    QSignalSpy removedSpy(model, &SlideModel::rowsRemoved);
    QSignalSpy dataChangedSpy(model, &SlideModel::dataChanged);

    // Add data
    model->setSlidePaths({m_dataDir.absolutePath()});
    spy.wait(10 * 1000);
    spy.wait(10 * 1000);
    spy.wait(10 * 1000);
    QCOMPARE(spy.size(), 3);
    spy.clear();

    QCOMPARE(model->rowCount(), 4);

    // Test unchecked items
    QPersistentModelIndex idx = model->index(0, 0);
    QCOMPARE(idx.data(ImageRoles::ToggleRole).toBool(), true);

    model->setData(idx, false, ImageRoles::ToggleRole);
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::ToggleRole);
    QCOMPARE(idx.data(ImageRoles::ToggleRole).toBool(), false);

    // Restore
    model->setData(idx, true, ImageRoles::ToggleRole);
    QCOMPARE(idx.data(ImageRoles::ToggleRole).toBool(), true);
    dataChangedSpy.clear();

    // Test setUncheckedSlides()
    model->setUncheckedSlides({idx.data(ImageRoles::PackageNameRole).toString()});
    QCOMPARE(idx.data(ImageRoles::ToggleRole).toBool(), false);

    // Test indexOf()
    QVERIFY(model->indexOf(m_dataDir.absoluteFilePath(QStringLiteral("wallpaper.jpg"))) >= 0);

    // Test addDirs()
    // Case 1: The url is a directory, and it doesn't exist in the model.
    QCOMPARE(model->addDirs({m_alternateDir.absolutePath()}), {m_alternateDir.absolutePath() + QDir::separator()});
    spy.wait(10 * 1000);
    spy.wait(10 * 1000);
    spy.wait(10 * 1000);
    QCOMPARE(spy.size(), 3);
    spy.clear();

    // Case 2: add again
    QCOMPARE(model->addDirs({m_alternateDir.absolutePath()}).size(), 0);

    // Case 3: add file, not directory
    QCOMPARE(model->addDirs({m_dataDir.absoluteFilePath(QStringLiteral("wallpaper.jpg"))}).size(), 0);

    QCOMPARE(model->rowCount(), 4 + 3);

    // Test removeDir()
    model->removeDir(m_alternateDir.absolutePath());
    QCOMPARE(removedSpy.size(), 1);

    QCOMPARE(model->rowCount(), 4);

    model->deleteLater();
}

void ModelTest::testSlideFilterModel()
{
    const auto filterModel = new SlideFilterModel(this);
    const auto model = new SlideModel(m_targetSize, filterModel);
    QSignalSpy spy(model, &SlideModel::rowsInserted);
    QSignalSpy filterSpy(filterModel, &SlideFilterModel::countChanged);
    QSignalSpy filterDataChangedSpy(filterModel, &SlideFilterModel::dataChanged);

    filterModel->setProperty("usedInConfig", false);
    filterModel->setSourceModel(model);

    model->setSlidePaths({m_dataDir.absolutePath()});
    spy.wait(10 * 1000);
    spy.wait(10 * 1000);
    spy.wait(10 * 1000);
    QCOMPARE(spy.size(), 3);
    spy.clear(); // Loaded

    QCOMPARE(filterSpy.size(), 3);
    filterSpy.clear();

    QCOMPARE(filterModel->rowCount(), 4);

    QPersistentModelIndex idx = filterModel->index(0, 0);
    QVERIFY(idx.isValid());

    // Test unchecked filter
    filterModel->setData(idx, false, ImageRoles::ToggleRole);
    QCOMPARE(filterDataChangedSpy.size(), 1);
    QCOMPARE(filterDataChangedSpy.takeFirst().at(2).value<QVector<int>>().constFirst(), ImageRoles::ToggleRole);
    QCOMPARE(idx.data(ImageRoles::ToggleRole).toBool(), false);

    filterModel->invalidateFilter();
    QCOMPARE(filterModel->rowCount(), 3);

    // If used in config, all wallpapers should be visible.
    filterModel->setProperty("usedInConfig", true);
    filterModel->invalidateFilter();
    QCOMPARE(filterModel->rowCount(), 4);

    // Test indexOf()
    QVERIFY(filterModel->indexOf(m_dataDir.absoluteFilePath(QStringLiteral("wallpaper.jpg"))) >= 0);

    filterModel->deleteLater();
}

QTEST_MAIN(ModelTest)

#include "test_model.moc"
