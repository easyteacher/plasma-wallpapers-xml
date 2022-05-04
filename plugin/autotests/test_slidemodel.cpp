/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDebug>
#include <QtTest>

#include "../finder/xmlfinder.h"
#include "../model/imageproxymodel.h"
#include "../slidemodel.h"

class SlideModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testSlideModelData();
    void testSlideModelIndexOf();
    void testSlideModelAddDirs();
    void testSlideModelRemoveDir();
    void testSlideModelSetSlidePaths();
    void testSlideModelSetUncheckedSlides();

private:
    QPointer<SlideModel> m_model = nullptr;
    QPointer<QSignalSpy> m_doneSpy = nullptr;
    QPointer<QSignalSpy> m_dataSpy = nullptr;

    QDir m_dataDir;
    QDir m_alternateDir;
    QString m_wallpaperPath;
    QString m_dummyWallpaperPath;
    QString m_packagePath;
    QString m_dummyPackagePath;
    QSize m_targetSize;
};

void SlideModelTest::initTestCase()
{
    qRegisterMetaType<QList<WallpaperItem>>();

    m_dataDir = QDir(QFINDTESTDATA("testdata/default"));
    m_alternateDir = QDir(QFINDTESTDATA("testdata/alternate"));
    QVERIFY(!m_dataDir.isEmpty());
    QVERIFY(!m_alternateDir.isEmpty());

    m_wallpaperPath = m_dataDir.absoluteFilePath(QStringLiteral("wallpaper.jpg.jpg"));
    m_dummyWallpaperPath = m_alternateDir.absoluteFilePath(QStringLiteral("dummy.jpg"));

    m_packagePath = m_dataDir.absoluteFilePath(QStringLiteral("package"));
    m_dummyPackagePath = m_alternateDir.absoluteFilePath(QStringLiteral("dummy"));

    m_targetSize = QSize(1920, 1080);

    QStandardPaths::setTestModeEnabled(true);
}

void SlideModelTest::init()
{
    m_model = new SlideModel(m_targetSize, this);
    m_doneSpy = new QSignalSpy(m_model, &SlideModel::done);
    m_dataSpy = new QSignalSpy(m_model, &SlideModel::dataChanged);

    // Test loading data
    m_model->setSlidePaths({m_dataDir.absolutePath()});

    QVERIFY(m_doneSpy->wait(10 * 1000));
    QCOMPARE(m_doneSpy->size(), 1);
    m_doneSpy->clear();
    QCOMPARE(m_model->sourceModels().size(), 1);
    QCOMPARE(m_model->m_models.size(), 1);
    QCOMPARE(m_model->rowCount(), 4); // wallpaper.jpg.jpg, package, two xml wallpapers

    QVERIFY(m_model->m_models.contains(m_dataDir.absolutePath() + QDir::separator()));
    QCOMPARE(m_model->m_models.value(m_dataDir.absolutePath() + QDir::separator())->count(), 4);
}

void SlideModelTest::cleanup()
{
    m_model->deleteLater();
    m_doneSpy->deleteLater();
    m_dataSpy->deleteLater();
}

void SlideModelTest::cleanupTestCase()
{
    const QString standardPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/wallpapers/");

    QDir(standardPath).removeRecursively();
}

void SlideModelTest::testSlideModelData()
{
    const int row = m_model->indexOf(m_packagePath);
    QVERIFY(row >= 0);
    const QPersistentModelIndex idx = m_model->index(row, 0);

    QCOMPARE(idx.data(ImageRoles::ToggleRole).toBool(), true);
    QCOMPARE(idx.data(ImageRoles::AuthorRole).toString(), QStringLiteral("Ken Vermette"));
    QCOMPARE(idx.data(ImageRoles::RemovableRole).toBool(), false);
    // Other roles are tested in ImageProxyModelTest
}

void SlideModelTest::testSlideModelIndexOf()
{
    QVERIFY(m_model->indexOf(m_wallpaperPath) >= 0);
    QVERIFY(m_model->indexOf(m_packagePath) >= 0);
    QVERIFY(m_model->indexOf(m_packagePath + QDir::separator()) >= 0);
    QCOMPARE(m_model->indexOf(m_dummyWallpaperPath), -1);
    QCOMPARE(m_model->indexOf(m_dummyPackagePath + QDir::separator()), -1);
}

void SlideModelTest::testSlideModelAddDirs()
{
    int count = m_model->rowCount();

    // Case 1: add an image file
    auto results = m_model->addDirs({m_wallpaperPath});
    QCOMPARE(m_model->m_models.size(), 1);
    QCOMPARE(results.size(), 0);

    // Case 2: add an existing folder
    results = m_model->addDirs({m_dataDir.absolutePath()});
    QCOMPARE(m_model->m_models.size(), 1);
    QCOMPARE(results.size(), 0);

    // Case 3: add a new folder
    results = m_model->addDirs({m_alternateDir.absolutePath()});
    QCOMPARE(results.size(), 1);
    QCOMPARE(m_model->m_models.size(), 2);
    QVERIFY(m_doneSpy->wait());
    QCOMPARE(m_doneSpy->size(), 1);
    m_doneSpy->clear();
    QCOMPARE(m_model->rowCount(), count + 3); // dummy.jpg, dummy package, dummy.xml
}

void SlideModelTest::testSlideModelRemoveDir()
{
    // Case 1: remove a not added dir
    m_model->removeDir(m_alternateDir.absolutePath());
    QCOMPARE(m_model->m_models.size(), 1);
    QCOMPARE(m_model->m_loaded, 1);
    QCOMPARE(m_model->rowCount(), 4);

    // Case 2: remove an existing dir
    m_model->removeDir(m_dataDir.absolutePath());
    QCOMPARE(m_model->m_models.size(), 0);
    QCOMPARE(m_model->m_loaded, 0);
    QCOMPARE(m_model->rowCount(), 0);
}

void SlideModelTest::testSlideModelSetSlidePaths()
{
    // Case 1: list is empty
    m_model->setSlidePaths({});
    QCOMPARE(m_model->m_models.size(), 0);
    QCOMPARE(m_model->m_loaded, 0);

    // Case 2: path is invalid
    m_model->setSlidePaths({QString()});
    QCOMPARE(m_model->m_models.size(), 0);
    QCOMPARE(m_model->m_loaded, 0);

    // Case 3: path is valid
    m_model->setSlidePaths({m_alternateDir.absolutePath()});
    QCOMPARE(m_model->m_models.size(), 1);
    QVERIFY(m_doneSpy->wait(10 * 1000));
    QCOMPARE(m_doneSpy->size(), 1);
    m_doneSpy->clear();
    QCOMPARE(m_model->sourceModels().size(), 1);
    QCOMPARE(m_model->m_models.size(), 1);
    QCOMPARE(m_model->rowCount(), 3);
}

void SlideModelTest::testSlideModelSetUncheckedSlides()
{
    QPersistentModelIndex idx = m_model->index(0, 0);

    QVERIFY(m_model->setData(idx, false, ImageRoles::ToggleRole));
    QCOMPARE(idx.data(ImageRoles::ToggleRole), false);

    QVERIFY(m_model->setData(idx, true, ImageRoles::ToggleRole));
    QCOMPARE(idx.data(ImageRoles::ToggleRole), true);
}

QTEST_MAIN(SlideModelTest)

#include "test_slidemodel.moc"
