/*
SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2

import org.kde.kirigami 2.12 as Kirigami

Item {
    anchors.fill: parent

    Kirigami.PlaceholderMessage {
        id: packageNotInstalledMessage

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        width: parent.width - (Kirigami.Units.largeSpacing * 4)

        icon.name: "install"
        text: i18n("Further Steps Required")
        explanation: i18n("Congratulations! You have just installed the plugin from the store. But to enable the plugin, you need to install an extra package. Please click the button below to see the installation tutorial.")
    }

    QQC2.Button {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: packageNotInstalledMessage.bottom
            topMargin: Kirigami.Units.largeSpacing
        }

        icon.name: "internet-web-browser"
        text: i18nc("@action:button", "Go to README.md")

        onClicked: Qt.openUrlExternally("https://github.com/easyteacher/plasma-wallpapers-xml/tree/main/README.md")
    }
}
