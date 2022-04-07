/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Window 2.0 // for Screen

import org.kde.kcm 1.5 as KCM
import org.kde.kirigami 2.12 as Kirigami

DropArea {
    KCM.GridView {
        id: wallpapersGrid

        anchors.fill: parent

        view.model: root.imageModel

        function resetCurrentIndex() {
            //that min is needed as the module will be populated in an async way
            //and only on demand so we can't ensure it already exists
            view.currentIndex = Qt.binding(() => Math.min(root.imageModel.indexOf(cfg_Image), root.imageModel.count - 1));
        }

        //kill the space for label under thumbnails
        Component.onCompleted: {
            resetCurrentIndex();
        }

        // Set the size of the cell, depending on Screen resolution to respect the aspect ratio
        view.implicitCellWidth: Screen.width / 10 + Kirigami.Units.smallSpacing * 2
        view.implicitCellHeight: Screen.height / 10 + Kirigami.Units.smallSpacing * 2 + Kirigami.Units.gridUnit * 3

        view.delegate: WallpaperDelegate {
            color: cfg_Color
        }
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        visible: wallpapersGrid.view.count === 0

        icon.name: !root.imageModel.loading ? "edit-none" : "image-loading-symbolic"
        text: !root.imageModel.loading ? i18n("No wallpapers") : i18n("Loading")
    }

    onEntered: {
        if (drag.hasUrls) {
            event.accept();
        }
    }

    onDropped: {
        drop.urls.forEach((url) => {
            if (url.indexOf("file://") === 0) {
                const path = url.substr(7); // 7 is length of "file://"
                root.imageModel.addBackground(path);
            }
        });
    }
}
