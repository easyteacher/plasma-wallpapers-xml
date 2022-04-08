/*
    SPDX-FileCopyrightText: 2007 Paolo Capriotti <p.capriotti@gmail.com>
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008 Petri Damsten <damu@iki.fi>
    SPDX-FileCopyrightText: 2008 Alexis Ménard <darktears31@gmail.com>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SLIDESHOWBACKEND_H
#define SLIDESHOWBACKEND_H

#include "imagebackend.h"
#include "model/sortingmode.h"

class SlideModel;
class SlideFilterModel;

/**
 * A backend for running slideshow
 */
class SlideshowBackend : public ImageBackend
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(SortingMode::Mode slideshowMode READ slideshowMode WRITE setSlideshowMode NOTIFY slideshowModeChanged)
    Q_PROPERTY(bool slideshowFoldersFirst READ slideshowFoldersFirst WRITE setSlideshowFoldersFirst NOTIFY slideshowFoldersFirstChanged)
    Q_PROPERTY(SlideFilterModel *slideFilterModel READ slideFilterModel CONSTANT)
    Q_PROPERTY(int slideTimer READ slideTimer WRITE setSlideTimer NOTIFY slideTimerChanged)
    Q_PROPERTY(QStringList slidePaths READ slidePaths WRITE setSlidePaths NOTIFY slidePathsChanged)
    Q_PROPERTY(QStringList uncheckedSlides READ uncheckedSlides WRITE setUncheckedSlides NOTIFY uncheckedSlidesChanged)

public:
    explicit SlideshowBackend(QObject *parent = nullptr);

    void classBegin() override;
    void componentComplete() override;

    SortingMode::Mode slideshowMode() const;
    void setSlideshowMode(SortingMode::Mode mode);

    bool slideshowFoldersFirst() const;
    void setSlideshowFoldersFirst(bool slideshowFoldersFirst);

    SlideFilterModel *slideFilterModel();

    int slideTimer() const;
    void setSlideTimer(int time);

    QStringList slidePaths() const;
    void setSlidePaths(const QStringList &slidePaths);

    QStringList uncheckedSlides() const;
    void setUncheckedSlides(const QStringList &uncheckedList);

    Q_INVOKABLE void setUrl(const QString &path) override;

    Q_INVOKABLE void removeDir(const QString &path);
    Q_INVOKABLE void showAddSlidePathsDialog();

    Q_INVOKABLE void openModelImage() const;

public Q_SLOTS:
    Q_INVOKABLE void nextSlide();

Q_SIGNALS:
    void slideshowModeChanged();
    void slideshowFoldersFirstChanged();
    void slideTimerChanged();
    void slidePathsChanged();
    void uncheckedSlidesChanged();

private Q_SLOTS:
    void startSlideshow();

    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    SlideModel *m_slideshowModel = nullptr;
    SlideFilterModel *m_slideFilterModel = nullptr;

    // Slideshow properties
    QStringList m_slidePaths;
    QStringList m_uncheckedSlides;
    int m_currentSlide = 0;
    SortingMode::Mode m_slideshowMode;
    bool m_slideshowFoldersFirst;

    QTimer m_timer; // Slideshow timer
    int m_delay = 0;
};

#endif // SLIDESHOWBACKEND_H
