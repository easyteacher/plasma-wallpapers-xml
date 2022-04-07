/*
    SPDX-FileCopyrightText: 2007 Paolo Capriotti <p.capriotti@gmail.com>
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008 Petri Damsten <damu@iki.fi>
    SPDX-FileCopyrightText: 2008 Alexis Ménard <darktears31@gmail.com>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "slideshowbackend.h"

#include <QFileDialog>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QUrlQuery>

#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KNotificationJobUiDelegate>
#include <KPackage/PackageLoader>

#include "model/slidefiltermodel.h"
#include "model/slidemodel.h"
#include "finder/packagefinder.h"

SlideshowBackend::SlideshowBackend(QObject *parent)
    : ImageBackend(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &SlideshowBackend::nextSlide);
}

void SlideshowBackend::classBegin()
{
}

void SlideshowBackend::componentComplete()
{
    m_ready = true;

    if (!m_usedInConfig) {
        connect(qGuiApp, &QGuiApplication::paletteChanged, this, &SlideshowBackend::slotUpdateXmlImage);
        startSlideshow();
    }
}

SortingMode::Mode SlideshowBackend::slideshowMode() const
{
    return m_slideshowMode;
}

void SlideshowBackend::setSlideshowMode(SortingMode::Mode slideshowMode)
{
    if (slideshowMode == m_slideshowMode) {
        return;
    }

    m_slideshowMode = slideshowMode;

    m_timer.stop();

    // Will trigger invalidate
    if (m_slideFilterModel) {
        m_slideFilterModel->setSortingMode(slideshowMode, m_slideshowFoldersFirst);
        m_slideFilterModel->sort(0);
    }

    startSlideshow();

    Q_EMIT slideshowModeChanged();
}

bool SlideshowBackend::slideshowFoldersFirst() const
{
    return m_slideshowFoldersFirst;
}

void SlideshowBackend::setSlideshowFoldersFirst(bool slideshowFoldersFirst)
{
    if (slideshowFoldersFirst == m_slideshowFoldersFirst) {
        return;
    }

    m_slideshowFoldersFirst = slideshowFoldersFirst;

    m_timer.stop();

    if (m_slideFilterModel) {
        m_slideFilterModel->setSortingMode(m_slideshowMode, m_slideshowFoldersFirst);
        m_slideFilterModel->sort(0);
    }

    Q_EMIT slideshowFoldersFirstChanged();
}

SlideFilterModel *SlideshowBackend::slideFilterModel()
{
    if (!m_slideshowModel) {
        m_slideshowModel = new SlideModel(m_targetSize, this);

        m_slideshowModel->setSlidePaths(m_slidePaths);
        m_slideshowModel->setUncheckedSlides(m_uncheckedSlides);

        connect(m_slideshowModel, &SlideModel::dataChanged, this, &SlideshowBackend::slotDataChanged);
        connect(this, &SlideshowBackend::targetSizeChanged, m_slideshowModel, &SlideModel::targetSizeChanged);
    }

    if (!m_slideFilterModel) {
        m_slideFilterModel = new SlideFilterModel(this);

        m_slideFilterModel->setProperty("usedInConfig", m_usedInConfig);
        m_slideFilterModel->setSourceModel(m_slideshowModel);
        m_slideFilterModel->setSortingMode(m_slideshowMode, m_slideshowFoldersFirst);
        m_slideFilterModel->sort(0);

        connect(this, &SlideshowBackend::uncheckedSlidesChanged, m_slideFilterModel, &SlideFilterModel::invalidateFilter);

        if (!m_usedInConfig) {
            connect(m_slideFilterModel, &QAbstractItemModel::modelReset, this, &SlideshowBackend::startSlideshow);
            connect(m_slideFilterModel, &QAbstractItemModel::rowsInserted, this, &SlideshowBackend::startSlideshow);
            connect(m_slideFilterModel, &QAbstractItemModel::rowsRemoved, this, &SlideshowBackend::startSlideshow);
        }
    }

    return m_slideFilterModel;
}

int SlideshowBackend::slideTimer() const
{
    return m_delay;
}

void SlideshowBackend::setSlideTimer(int time)
{
    if (time == m_delay) {
        return;
    }

    m_delay = time;

    m_timer.stop();
    startSlideshow();

    Q_EMIT slideTimerChanged();
}

QStringList SlideshowBackend::slidePaths() const
{
    return m_slidePaths;
}

void SlideshowBackend::setSlidePaths(const QStringList &_slidePaths)
{
    if (_slidePaths == m_slidePaths) {
        return;
    }

    QStringList slidePaths = _slidePaths;
    slidePaths.removeAll(QString());

    if (!slidePaths.empty()) {
        // Replace 'preferred://wallpaperlocations' with real paths
        const QStringList preProcessedPaths = slidePaths;

        for (const QString &path : preProcessedPaths) {
            if (path == QLatin1String("preferred://wallpaperlocations")) {
                slidePaths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("wallpapers"), QStandardPaths::LocateDirectory);
                slidePaths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("gnome-background-properties"), QStandardPaths::LocateDirectory);;
                slidePaths.removeAll(path);

                break;
            }
        }
    }

    if (slidePaths == m_slidePaths) {
        return;
    }

    m_slidePaths = slidePaths;

    m_timer.stop();

    if (m_slideshowModel) {
        m_slideshowModel->setSlidePaths(m_slidePaths); // Will trigger rowsRemoved and rowsInserted
    }

    Q_EMIT slidePathsChanged();
}

QStringList SlideshowBackend::uncheckedSlides() const
{
    return m_uncheckedSlides;
}

void SlideshowBackend::setUncheckedSlides(const QStringList &uncheckedSlides)
{
    if (uncheckedSlides == m_uncheckedSlides) {
        return;
    }

    m_uncheckedSlides = uncheckedSlides;

    m_timer.stop();

    if (m_slideshowModel) {
        m_slideshowModel->setUncheckedSlides(m_uncheckedSlides);
    }

    Q_EMIT uncheckedSlidesChanged(); // Will trigger invalidateFilter
}

void SlideshowBackend::removeDir(const QString &path)
{
    m_slideshowModel->removeDir(path);

    if (m_slidePaths.removeOne(path)) {
        Q_EMIT slidePathsChanged();
    }
}

void SlideshowBackend::showAddSlidePathsDialog()
{
    if (!m_dialog) {
        m_dialog = new QFileDialog(nullptr, i18n("Directory with the wallpaper to show slides from"), QString());

        m_dialog->setOptions(QFileDialog::ShowDirsOnly);
        m_dialog->setAcceptMode(QFileDialog::AcceptOpen);

        connect(m_dialog, &QDialog::accepted, this, [this]{
            const QString dir = m_dialog->directoryUrl().toLocalFile();

            m_slidePaths.append(m_slideshowModel->addDirs({dir}));
            Q_EMIT slidePathsChanged();

            m_dialog->deleteLater();
            m_dialog = nullptr;
        });
    }

    m_dialog->show();
}

void SlideshowBackend::openModelImage() const
{
    QUrl url;

    switch(m_providerType) {
    case Provider::Image: {
        url = m_image;
        break;
    }

    case Provider::Package: {
        KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Wallpaper/Images"));
        package.setPath(m_image.toLocalFile());

        if (!package.isValid()) {
            return;
        }

        findPreferredImageInPackage(package, m_targetSize);
        url = QUrl::fromLocalFile(package.filePath("preferred"));
        break;
    }

    case Provider::Xml: {
        const QUrlQuery urlQuery(m_modelImage);

        const QString filename = urlQuery.queryItemValue(QStringLiteral("filename"));
        const QString filename_dark = urlQuery.queryItemValue(QStringLiteral("filename_dark"));
        const bool useDark = urlQuery.queryItemValue(QStringLiteral("darkmode")).toInt() == 1;

        // Avoid processing slideshow here because it's trivial
        url = QUrl::fromLocalFile(useDark && !filename_dark.isEmpty() ? filename_dark : filename);
        break;
    }
    }

    KIO::OpenUrlJob *job = new KIO::OpenUrlJob(url);
    job->setUiDelegate(new KNotificationJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled));
    job->start();
}

void SlideshowBackend::startSlideshow()
{
    if (!m_ready || m_usedInConfig) {
        return;
    }

    if (m_timer.isActive()) {
        // The slideshow is already running.
        return;
    }

    // Will create a new filter model if it doesn't exist
    if (slideFilterModel()->rowCount() == 0) {
        // No image has been found after the model is just created, wait for rowsInserted signal.
        return;
    }

    // start slideshow
    if (m_currentSlide == -1) {
        m_currentSlide = m_slideFilterModel->indexOf(m_image.toString()) - 1;
    } else {
        m_currentSlide = -1;
    }

    nextSlide();
    // TODO: what would be cool: paint on the wallpaper itself a busy widget and perhaps some text
    // about loading wallpaper slideshow while the thread runs
}

void SlideshowBackend::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    Q_UNUSED(bottomRight);

    if (!topLeft.isValid()) {
        return;
    }

    if (roles.contains(ImageRoles::ToggleRole)) {
        if (topLeft.data(ImageRoles::ToggleRole).toBool()) {
            m_uncheckedSlides.removeOne(topLeft.data(ImageRoles::PackageNameRole).toString());
        } else {
            m_uncheckedSlides.append(topLeft.data(ImageRoles::PackageNameRole).toString());
        }

        Q_EMIT uncheckedSlidesChanged();
    }
}

void SlideshowBackend::nextSlide()
{
    const int rowCount = m_slideFilterModel->rowCount();

    if (!m_ready || m_delay < 1 || rowCount == 0) {
        return;
    }

    // When timer is running, startSlideshow will directly return to avoid dead loop.
    m_timer.start(m_delay * 1000);

    int previousSlide = m_currentSlide;

    QUrl previousPath = m_slideFilterModel->index(m_currentSlide, 0).data(ImageRoles::PackageNameRole).toString();


    if (m_currentSlide < 0 || m_currentSlide == rowCount - 1 || m_currentSlide >= rowCount) {
        m_currentSlide = 0;
    } else {
        m_currentSlide += 1;
    }

    // We are starting again - avoid having the same random order when we restart the slideshow
    if (m_slideshowMode == SortingMode::Random && m_currentSlide == 0) {
        m_slideFilterModel->invalidate();
    }

    QUrl next = m_slideFilterModel->index(m_currentSlide, 0).data(ImageRoles::PackageNameRole).toString();
    // And avoid showing the same picture twice
    if (previousSlide == rowCount - 1 && previousPath == next && rowCount > 1) {
        m_currentSlide += 1;
        next = m_slideFilterModel->index(m_currentSlide, 0).data(ImageRoles::PackageNameRole).toString();
    }

    if (next.isEmpty()) {
        m_image = previousPath;
    } else {
        m_image = next;
    }

    Q_EMIT imageChanged();

    setModelImage();
}
