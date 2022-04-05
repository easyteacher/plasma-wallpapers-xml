/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2

import org.kde.kcm 1.5 as KCM

QQC2.ComboBox {
    id: resizeComboBox

    model: [
        {
            'label': i18nd("plasma_wallpaper_org.kde.image", "Scaled and Cropped"),
            'fillMode': Image.PreserveAspectCrop
        },
        {
            'label': i18nd("plasma_wallpaper_org.kde.image", "Scaled"),
            'fillMode': Image.Stretch
        },
        {
            'label': i18nd("plasma_wallpaper_org.kde.image", "Scaled, Keep Proportions"),
            'fillMode': Image.PreserveAspectFit
        },
        {
            'label': i18nd("plasma_wallpaper_org.kde.image", "Centered"),
            'fillMode': Image.Pad
        },
        {
            'label': i18nd("plasma_wallpaper_org.kde.image", "Tiled"),
            'fillMode': Image.Tile
        },
    ]

    textRole: "label"
    onActivated: cfg_FillMode = model[currentIndex]["fillMode"]
    Component.onCompleted: setMethod()

    KCM.SettingHighlighter {
        highlight: cfg_FillModeDefault != cfg_FillMode
    }

    function setMethod() {
        for (let i = 0; i < model.length; i++) {
            if (model[i]["fillMode"] === cfg_FillMode) {
                resizeComboBox.currentIndex = i;
                break;
            }
        }
    }
}
