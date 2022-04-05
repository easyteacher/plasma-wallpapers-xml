/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2

import org.kde.kcm 1.5 as KCM

import com.github.easyteacher.plasma.wallpapers.xml 1.0

Row {
    id: slideshowModeRow

    QQC2.ComboBox {
        id: slideshowModeComboBox

        model: [
            {
                'label': i18nd("plasma_wallpaper_org.kde.image", "Random"),
                'slideshowMode':  SortingMode.Random
            },
            {
                'label': i18nd("plasma_wallpaper_org.kde.image", "A to Z"),
                'slideshowMode':  SortingMode.Alphabetical
            },
            {
                'label': i18nd("plasma_wallpaper_org.kde.image", "Z to A"),
                'slideshowMode':  SortingMode.AlphabeticalReversed
            },
            {
                'label': i18nd("plasma_wallpaper_org.kde.image", "Date modified (newest first)"),
                'slideshowMode':  SortingMode.ModifiedReversed
            },
            {
                'label': i18nd("plasma_wallpaper_org.kde.image", "Date modified (oldest first)"),
                'slideshowMode':  SortingMode.Modified
            }
        ]

        textRole: "label"
        onActivated: cfg_SlideshowMode = model[currentIndex]["slideshowMode"]
        Component.onCompleted: slideshowModeRow.setMethod()

        KCM.SettingHighlighter {
            highlight: cfg_SlideshowMode != cfg_SlideshowModeDefault
        }
    }

    QQC2.CheckBox {
        id: slideshowFoldersFirstCheckBox
        anchors.verticalCenter: slideshowModeComboBox.verticalCenter
        text: i18nd("plasma_wallpaper_org.kde.image", "Group by folders")
        checked: cfg_SlideshowFoldersFirst
        onToggled: cfg_SlideshowFoldersFirst = slideshowFoldersFirstCheckBox.checked

        KCM.SettingHighlighter {
            highlight: cfg_SlideshowFoldersFirst !== cfg_SlideshowFoldersFirstDefault
        }
    }

    function setMethod() {
        for (let i = 0; i < slideshowModeComboBox.model.length; i++) {
            if (slideshowModeComboBox.model[i]["slideshowMode"] === cfg_SlideshowMode) {
                slideshowModeComboBox.currentIndex = i;
                break;
            }
        }
    }
}
