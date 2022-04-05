/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2

import org.kde.kcm 1.5 as KCM
import org.kde.kquickcontrols 2.0 as KQuickControls

Column {
    QQC2.ButtonGroup {
        id: backgroundGroup
    }

    QQC2.RadioButton {
        id: blurRadioButton
        checked: cfg_Blur
        text: i18nd("plasma_wallpaper_org.kde.image", "Blur")
        QQC2.ButtonGroup.group: backgroundGroup

        onToggled: cfg_Blur = checked

        KCM.SettingHighlighter {
            highlight: cfg_Blur != cfg_BlurDefault
        }
    }

    Row {
        id: colorRow
        spacing: parent.spacing

        QQC2.RadioButton {
            id: colorRadioButton
            anchors.verticalCenter: colorButton.verticalCenter
            text: i18nd("plasma_wallpaper_org.kde.image", "Solid color")
            checked: !cfg_Blur
            QQC2.ButtonGroup.group: backgroundGroup

            onToggled: cfg_Blur = !checked

            KCM.SettingHighlighter {
                highlight: cfg_Blur != cfg_BlurDefault
            }
        }

        KQuickControls.ColorButton {
            id: colorButton
            dialogTitle: i18nd("plasma_wallpaper_org.kde.image", "Select Background Color")

            color: cfg_Color
            onColorChanged: cfg_Color = color

            KCM.SettingHighlighter {
                highlight: cfg_Color != cfg_ColorDefault
            }
        }
    }
}
