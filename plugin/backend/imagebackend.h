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

#ifndef IMAGEBACKEND_H
#define IMAGEBACKEND_H

#include <QObject>
#include <QQmlParserStatus>
#include <QSize>
#include <QUrl>

#include "xmlslideshowupdatetimer.h"

class QFileDialog;
class ImageProxyModel;
class KJob;

/**
 * A backend for providing wallpaper urls.
 */
class ImageBackend : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QSize targetSize READ targetSize WRITE setTargetSize NOTIFY targetSizeChanged)
    Q_PROPERTY(bool usedInConfig MEMBER m_usedInConfig NOTIFY usedInConfigChanged)

    /**
     * Package path from the saved configuration, can be an image file, a url with
     * "image://" scheme or a folder (KPackage).
     */
    Q_PROPERTY(QUrl image READ image WRITE setImage NOTIFY imageChanged)
    /**
     * The real path of the image
     * e.g. /home/kde/Pictures/image.png
     *      image://gnome-wp-list/get? (XML)
     *      image://package/get? (KPackage)
     */
    Q_PROPERTY(QUrl modelImage READ modelImage NOTIFY modelImageChanged)
    /**
     * Transition type wallpaper
     */
    Q_PROPERTY(bool isTransition READ isTransition NOTIFY isTransitionChanged)

    Q_PROPERTY(ImageProxyModel *imageModel READ imageModel CONSTANT)

public:
    enum class Provider {
        Image,
        Package,
        Xml,
    };
    Q_ENUM(Provider)

    explicit ImageBackend(QObject *parent = nullptr);
    ~ImageBackend();

    virtual void classBegin() override;
    virtual void componentComplete() override;

    QUrl image() const;
    void setImage(const QUrl &url);

    /**
     * @return The real path of the image
     */
    QUrl modelImage() const;

    bool isTransition() const;

    QSize targetSize() const;
    void setTargetSize(const QSize &size);

    Q_INVOKABLE void useDefaultImage();

    ImageProxyModel *imageModel();
    void releaseImageModel();

    Q_INVOKABLE virtual void setUrl(const QString &path);
    Q_INVOKABLE void showFileDialog();

Q_SIGNALS:
    void usedInConfigChanged();

    void imageChanged();
    void modelImageChanged();
    void isTransitionChanged();

    void targetSizeChanged(const QSize &size);

protected:
    void setModelImage();

    bool m_ready = false;
    bool m_usedInConfig = true;

    QSize m_targetSize;

    Provider m_providerType;
    QUrl m_image;
    QUrl m_modelImage;

    QFileDialog *m_dialog = nullptr;

protected Q_SLOTS:
    void slotUpdateXmlImage(const QPalette &palette);
    void slotWallpaperBrowseCompleted();
    void slotCopyWallpaperResult(KJob *job);

private:
    ImageProxyModel *m_imageModel = nullptr;

    XmlSlideshowUpdateTimer m_xmlTimer;
    QMetaObject::Connection m_changeConnection;
    QMetaObject::Connection m_resumeConnection;
};

#endif // IMAGEBACKEND_H
