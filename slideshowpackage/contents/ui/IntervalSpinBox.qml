/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2

import org.kde.kcm 1.5 as KCM

Row {
    id: intervalSpinBox

    property int hoursIntervalValue: Math.floor(cfg_SlideInterval / 3600)
    property int minutesIntervalValue: Math.floor(cfg_SlideInterval % 3600) / 60
    property int secondsIntervalValue: cfg_SlideInterval % 3600 % 60

    property int hoursIntervalValueDefault: Math.floor(cfg_SlideIntervalDefault / 3600)
    property int minutesIntervalValueDefault: Math.floor(cfg_SlideIntervalDefault % 3600) / 60
    property int secondsIntervalValueDefault: cfg_SlideIntervalDefault % 3600 % 60

    onHoursIntervalValueChanged: hoursInterval.value = hoursIntervalValue
    onMinutesIntervalValueChanged: minutesInterval.value = minutesIntervalValue
    onSecondsIntervalValueChanged: secondsInterval.value = secondsIntervalValue

    QQC2.SpinBox {
        id: hoursInterval
        value: intervalSpinBox.hoursIntervalValue
        from: 0
        to: 24
        editable: true
        onValueChanged: cfg_SlideInterval = hoursInterval.value * 3600 + minutesInterval.value * 60 + secondsInterval.value

        textFromValue: (value, locale) => i18ndp("plasma_wallpaper_org.kde.image","%1 hour", "%1 hours", value)
        valueFromText: (text, locale) => parseInt(text)

        KCM.SettingHighlighter {
            highlight: intervalSpinBox.hoursIntervalValue != intervalSpinBox.hoursIntervalValueDefault
        }
    }

    QQC2.SpinBox {
        id: minutesInterval
        value: intervalSpinBox.minutesIntervalValue
        from: 0
        to: 60
        editable: true
        onValueChanged: cfg_SlideInterval = hoursInterval.value * 3600 + minutesInterval.value * 60 + secondsInterval.value

        textFromValue: (value, locale) => i18ndp("plasma_wallpaper_org.kde.image","%1 minute", "%1 minutes", value)
        valueFromText: (text, locale) => parseInt(text)

        KCM.SettingHighlighter {
            highlight: intervalSpinBox.minutesIntervalValue != intervalSpinBox.minutesIntervalValueDefault
        }
    }

    QQC2.SpinBox {
        id: secondsInterval
        value: intervalSpinBox.secondsIntervalValue
        from: intervalSpinBox.hoursIntervalValue === 0 && intervalSpinBox.minutesIntervalValue === 0 ? 1 : 0
        to: 60
        editable: true
        onValueChanged: cfg_SlideInterval = hoursInterval.value * 3600 + minutesInterval.value * 60 + secondsInterval.value

        textFromValue: (value, locale) => i18ndp("plasma_wallpaper_org.kde.image","%1 second", "%1 seconds", value)
        valueFromText: (text, locale) => parseInt(text)

        KCM.SettingHighlighter {
            highlight: intervalSpinBox.secondsIntervalValue != intervalSpinBox.secondsIntervalValueDefault
        }
    }
}
