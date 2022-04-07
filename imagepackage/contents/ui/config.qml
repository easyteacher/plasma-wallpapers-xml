/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5

Item {
    id: fakeRoot

    property bool cfg_Blur
    property bool cfg_BlurDefault

    property color cfg_Color
    property color cfg_ColorDefault

    property string cfg_Image
    property string cfg_ImageDefault

    property int cfg_FillMode
    property int cfg_FillModeDefault

    property int cfg_SlideshowMode
    property int cfg_SlideshowModeDefault

    property bool cfg_SlideshowFoldersFirst
    property bool cfg_SlideshowFoldersFirstDefault: false

    property var cfg_SlidePaths: []
    property var cfg_SlidePathsDefault: []

    property int cfg_SlideInterval: 0
    property int cfg_SlideIntervalDefault: 0

    property var cfg_UncheckedSlides: []
    property var cfg_UncheckedSlidesDefault: []

    function saveConfig() {
        if (configDialog.currentWallpaper === "com.github.easyteacher.plasma.wallpapers.xml") {
            generalLoader.item.imageModel.commitAddition();
            generalLoader.item.imageModel.commitDeletion();
        }
    }

    Loader {
        id: generalLoader
        anchors.fill: parent
        asynchronous: true

        source: "ConfigGeneral.qml"
    }
}
