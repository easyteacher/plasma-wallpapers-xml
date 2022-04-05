/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0
import QtQuick.Window 2.0 // for Screen

import org.kde.kirigami 2.5 as Kirigami
import org.kde.newstuff 1.91 as NewStuff

import com.github.easyteacher.plasma.wallpapers.xml 1.0

ColumnLayout {
    id: root

    anchors.fill: parent

    property alias imageModel: backend.imageModel

    ImageBackend {
        id: backend
        targetSize: {
            if (typeof plasmoid !== "undefined") {
                return Qt.size(plasmoid.width, plasmoid.height);
            }
            // Lock screen configuration case
            return Qt.size(root.width * Screen.devicePixelRatio, root.height * Screen.devicePixelRatio);
        }
    }

    Kirigami.FormLayout {
        twinFormLayouts: parentLayout

        ResizeComboBox {
            id: resizeComboBox

            Layout.preferredWidth: Math.max(implicitWidth, wallpaperComboBox.implicitWidth)
            Kirigami.FormData.label: i18nd("plasma_wallpaper_org.kde.image", "Positioning:")
        }

        ColorOption {
            spacing: Kirigami.Units.smallSpacing
            Kirigami.FormData.label: i18nd("plasma_wallpaper_org.kde.image", "Background:")
            visible: cfg_FillMode === Image.PreserveAspectFit || cfg_FillMode === Image.Pad
        }
    }

    WallpaperGridView {
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    RowLayout {
        id: buttonsRow

        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

        QQC2.Button {
            icon.name: "list-add"
            text: i18nd("plasma_wallpaper_org.kde.image", "Add Image…")
            onClicked: backend.showFileDialog();
        }

        NewStuff.Button {
            Layout.alignment: Qt.AlignRight

            configFile: Kirigami.Settings.isMobile ? "wallpaper-mobile.knsrc" : "wallpaper.knsrc"
            viewMode: NewStuff.Page.ViewMode.Preview

            text: i18nd("plasma_wallpaper_org.kde.image", "Get New Wallpapers…")

            onEntryEvent: (entry, event) => {
                if (event == NewStuff.Entry.StatusChangedEvent) {
                    backend.imageModel.reload();
                }
            }
        }
    }
}
