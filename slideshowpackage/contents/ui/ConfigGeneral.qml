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

import org.kde.kirigami 2.12 as Kirigami
import org.kde.newstuff 1.91 as NewStuff

import com.github.easyteacher.plasma.wallpapers.xml 1.0

ColumnLayout {
    id: root

    anchors.fill: parent

    property alias imageModel: backend.slideFilterModel

    SlideshowBackend {
        id: backend
        targetSize: {
            if (typeof plasmoid !== "undefined") {
                return Qt.size(plasmoid.width, plasmoid.height);
            }
            // Lock screen configuration case
            return Qt.size(root.width * Screen.devicePixelRatio, root.height * Screen.devicePixelRatio);
        }

        slidePaths: cfg_SlidePaths
        uncheckedSlides: cfg_UncheckedSlides
        slideshowMode: cfg_SlideshowMode
        slideshowFoldersFirst: cfg_SlideshowFoldersFirst

        onSlidePathsChanged: cfg_SlidePaths = slidePaths // After adding a folder
        onUncheckedSlidesChanged: cfg_UncheckedSlides = uncheckedSlides
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

        SlideshowMode {
            id: slideshowModeRow
            Kirigami.FormData.label: i18nd("plasma_wallpaper_org.kde.image", "Order:")
            spacing: Kirigami.Units.smallSpacing
        }

        IntervalSpinBox {
            Kirigami.FormData.label: i18nd("plasma_wallpaper_org.kde.image", "Change every:")
            spacing: Kirigami.Units.smallSpacing
        }
    }

    Kirigami.Heading {
        text: i18nd("plasma_wallpaper_org.kde.image", "Folders")
        level: 2
    }

    GridLayout {
        columns: 2
        Layout.fillWidth: true
        Layout.fillHeight: true
        columnSpacing: Kirigami.Units.largeSpacing

        QQC2.ScrollView {
            id: foldersScroll
            Layout.fillHeight: true
            Layout.preferredWidth: 0.35 * parent.width
            Layout.maximumWidth: Kirigami.Units.gridUnit * 16

            Component.onCompleted: foldersScroll.background.visible = true;

            ListView {
                id: slidePathsView

                model: backend.slidePaths

                delegate: Kirigami.SwipeListItem {
                    width: slidePathsView.width
                    // content item includes its own padding
                    padding: 0
                    // Don't need a highlight or hover effects
                    hoverEnabled: false
                    contentItem: Kirigami.BasicListItem {
                        // Don't need a highlight or hover effects
                        hoverEnabled: false
                        separatorVisible: false

                        // Header: the folder
                        label: {
                            var strippedPath = modelData.replace(/\/+$/, "");
                            return strippedPath.split('/').pop()
                        }
                        // Subtitle: the path to the folder
                        subtitle: {
                            var strippedPath = modelData.replace(/\/+$/, "");
                            return strippedPath.replace(/\/[^\/]*$/, '');;
                        }

                        QQC2.ToolTip.text: modelData
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: 1000
                        QQC2.ToolTip.timeout: 5000
                    }

                    actions: [
                        Kirigami.Action {
                            iconName: "list-remove"
                            tooltip: i18nd("plasma_wallpaper_org.kde.image", "Remove Folder")
                            onTriggered: backend.removeDir(modelData)
                        },
                        Kirigami.Action {
                            icon.name: "document-open-folder"
                            tooltip: i18nd("plasma_wallpaper_org.kde.image", "Open Folder")
                            onTriggered: Qt.openUrlExternally(modelData)
                        }
                    ]
                }

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: slidePathsView.count === 0
                    text: i18nd("plasma_wallpaper_org.kde.image", "There are no wallpaper locations configured")
                }
            }
        }

        WallpaperGridView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        QQC2.Button {
            Layout.alignment: Qt.AlignRight
            icon.name: "list-add"
            text: i18nd("plasma_wallpaper_org.kde.image","Add Folder…")
            onClicked: backend.showAddSlidePathsDialog()
        }

        NewStuff.Button {
            Layout.alignment: Qt.AlignRight
            configFile: Kirigami.Settings.isMobile ? "wallpaper-mobile.knsrc" : "wallpaper.knsrc"
            text: i18nd("plasma_wallpaper_org.kde.image", "Get New Wallpapers…")
            viewMode: NewStuff.Page.ViewMode.Preview
        }
    }
}
