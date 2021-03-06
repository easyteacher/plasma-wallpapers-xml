/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.1 as QQC2
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import com.github.easyteacher.plasma.wallpapers.xml 2.0 as Wallpaper
import org.kde.plasma.core 2.0 as PlasmaCore

QQC2.StackView {
    id: root

    readonly property int fillMode: wallpaper.configuration.FillMode
    readonly property string configColor: wallpaper.configuration.Color
    readonly property bool blur: wallpaper.configuration.Blur
    readonly property size sourceSize: Qt.size(root.width * Screen.devicePixelRatio, root.height * Screen.devicePixelRatio)

    // Ppublic API functions accessible from C++:

    // e.g. used by WallpaperInterface for drag and drop
    function setUrl(url) {
        if (wallpaper.pluginName === "com.github.easyteacher.plasma.wallpapers.xml") {
            const result = imageWallpaper.addUsersWallpaper(url);

            if (result.length > 0) {
                // Can be a file or a folder (KPackage)
                wallpaper.configuration.Image = result;
            }
        } else {
            imageWallpaper.addSlidePath(url);
        }
    }

    // e.g. used by slideshow wallpaper plugin
    function action_next() {
        imageWallpaper.nextSlide();
    }

    // e.g. used by slideshow wallpaper plugin
    function action_open() {
        imageWallpaper.openModelImage();
    }

    //private

    Component.onCompleted: {
        wallpaper.loading = true; // delays ksplash until the wallpaper has been loaded

        if (wallpaper.pluginName === "com.github.easyteacher.plasma.wallpapers.xml.slideshow") {
            wallpaper.setAction("open", i18nd("plasma_wallpaper_org.kde.image", "Open Wallpaper Image"), "document-open");
            wallpaper.setAction("next", i18nd("plasma_wallpaper_org.kde.image", "Next Wallpaper Image"), "user-desktop");
        }
    }

    Wallpaper.ImageBackend {
        id: imageWallpaper
        usedInConfig: false
        //the oneliner of difference between image and slideshow wallpapers
        renderingMode: (wallpaper.pluginName === "com.github.easyteacher.plasma.wallpapers.xml") ? Wallpaper.ImageBackend.SingleImage : Wallpaper.ImageBackend.SlideShow
        image: wallpaper.pluginName === "com.github.easyteacher.plasma.wallpapers.xml" ? wallpaper.configuration.Image : ""
        targetSize: root.sourceSize
        slidePaths: wallpaper.configuration.SlidePaths
        slideTimer: wallpaper.configuration.SlideInterval
        slideshowMode: wallpaper.configuration.SlideshowMode
        slideshowFoldersFirst: wallpaper.configuration.SlideshowFoldersFirst
        uncheckedSlides: wallpaper.configuration.UncheckedSlides

        // Update wallpaper after resume from sleep
        onModelImageChanged: Qt.callLater(loadImage);
        // Save drag and drop result
        onSlidePathsChanged: wallpaper.configuration.SlidePaths = slidePaths
    }

    onFillModeChanged: Qt.callLater(loadImage);
    onConfigColorChanged: Qt.callLater(loadImage);
    onBlurChanged: Qt.callLater(loadImage);

    function loadImage() {
        var doesSkipAnimation = root.empty || imageWallpaper.isTransition;
        var pendingImage = baseImage.createObject(root, {
                        "source": imageWallpaper.modelImage,
                        "fillMode": root.fillMode,
                        "sourceSize": root.sourceSize,
                        "color": root.configColor,
                        "blur": root.blur,
                        "opacity": doesSkipAnimation ? 1: 0});

        function replaceWhenLoaded() {
            if (pendingImage.status !== Image.Loading) {
                root.replace(pendingImage, {},
                    doesSkipAnimation ? QQC2.StackView.Immediate : QQC2.StackView.Transition);//dont' animate first show
                pendingImage.statusChanged.disconnect(replaceWhenLoaded);

                wallpaper.loading = false;

                if (pendingImage.status !== Image.Ready) {
                    imageWallpaper.useSingleImageDefaults();
                }
            }
        }
        pendingImage.statusChanged.connect(replaceWhenLoaded);
        replaceWhenLoaded();
    }

    Component {
        id: baseImage

        Image {
            id: mainImage

            property alias color: backgroundColor.color
            property bool blur: false

            asynchronous: true
            cache: false
            autoTransform: true
            z: -1
            layer.enabled: true

            QQC2.StackView.onActivated: mainImage.layer.enabled = false
            QQC2.StackView.onRemoved: destroy()

            Rectangle {
                id: backgroundColor
                anchors.fill: parent
                visible: mainImage.status === Image.Ready && !blurLoader.active
                z: -2
            }

            Loader {
                id: blurLoader
                anchors.fill: parent
                z: -3
                active: mainImage.blur && (mainImage.fillMode === Image.PreserveAspectFit || mainImage.fillMode === Image.Pad)
                sourceComponent: Item {
                    Image {
                        id: blurSource
                        anchors.fill: parent
                        asynchronous: true
                        cache: false
                        autoTransform: true
                        fillMode: Image.PreserveAspectCrop
                        source: mainImage.source
                        sourceSize: mainImage.sourceSize
                        visible: false // will be rendered by the blur
                    }

                    GaussianBlur {
                        id: blurEffect
                        anchors.fill: parent
                        source: blurSource
                        radius: 32
                        samples: 65
                        visible: blurSource.status === Image.Ready
                    }
                }
            }
        }
    }

    replaceEnter: Transition {
        OpacityAnimator {
            from: 0
            to: 1
            duration: wallpaper.configuration.TransitionAnimationDuration
        }
    }
    // Keep the old image around till the new one is fully faded in
    // If we fade both at the same time you can see the background behind glimpse through
    replaceExit: Transition{
        PauseAnimation {
            duration: wallpaper.configuration.TransitionAnimationDuration
        }
    }
}
