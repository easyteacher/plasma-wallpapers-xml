/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Window 2.15

import com.github.easyteacher.plasma.wallpapers.xml 2.0

QQC2.StackView {
    id: root

    readonly property bool blur: wallpaper.configuration.Blur
    readonly property string configColor: wallpaper.configuration.Color
    readonly property int fillMode: wallpaper.configuration.FillMode

    readonly property size targetSize: Qt.size(root.width * Screen.devicePixelRatio, root.height * Screen.devicePixelRatio)

    ImageBackend {
        id: backend

        image: wallpaper.configuration.Image
        targetSize: root.targetSize
        usedInConfig: false

        onModelImageChanged: Qt.callLater(loadImage)
        onImageChanged: wallpaper.configuration.Image = image
    }

    /**
     * e.g. used by WallpaperInterface for drag and drop
     */
    function setUrl(url) {
        backend.setUrl(url);
    }

    function loadImage() {
        const baseImage = Qt.createComponent("ImageComponent.qml");

        if (baseImage.status !== Component.Ready) {
            return;
        }

        const pendingImage = baseImage.createObject(root, {
            "source": backend.modelImage,
            "fillMode": root.fillMode,
            "sourceSize": root.targetSize,
            "color": root.configColor,
            "blur": root.blur,
            "opacity": root.empty || backend.isTransition ? 1: 0,
        });

        function replaceWhenLoaded() {
            if (pendingImage.status !== Image.Loading) {
                //dont' animate first show
                root.replace(pendingImage, {}, root.empty || backend.isTransition ? QQC2.StackView.Immediate : QQC2.StackView.Transition);
                pendingImage.statusChanged.disconnect(replaceWhenLoaded);

                wallpaper.loading = false;

                if (pendingImage.status !== Image.Ready) {
                    backend.useDefaultImage();
                }
            }
        }

        pendingImage.statusChanged.connect(replaceWhenLoaded);
        replaceWhenLoaded();
    }

    onConfigColorChanged: Qt.callLater(loadImage)
    onFillModeChanged: Qt.callLater(loadImage)
    onBlurChanged: Qt.callLater(loadImage)

    replaceEnter: Transition {
        OpacityAnimator {
            from: 0
            to: 1
            duration: wallpaper.configuration.TransitionAnimationDuration || 1000
            easing.type: Easing.OutCubic
        }
    }

    // Keep the old image around till the new one is fully faded in
    // If we fade both at the same time you can see the background behind glimpse through
    replaceExit: Transition{
        PauseAnimation {
            duration: wallpaper.configuration.TransitionAnimationDuration || 1000
        }
    }

    Component.onCompleted: {
        wallpaper.loading = true; // delays ksplash until the wallpaper has been loaded
    }
}
