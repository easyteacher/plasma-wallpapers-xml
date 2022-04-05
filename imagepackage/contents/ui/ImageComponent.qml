/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtGraphicalEffects 1.0

Image {
    id: mainImage

    property alias color: backgroundColor.color
    property bool blur: false

    asynchronous: true
    cache: false
    autoTransform: true
    z: -1

    QQC2.StackView.onRemoved: {
        source = "";  // HACK: Release more memory
        destroy();
    }

    Rectangle {
        id: backgroundColor
        anchors.fill: parent
        visible: mainImage.status === Image.Ready && !blurLoader.active
              && (mainImage.fillMode === Image.PreserveAspectFit || mainImage.fillMode === Image.Pad)
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

