import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.5
import JOIP.db 1.5
import JOIP.ui 1.1

Rectangle {
    id: achievementItem
    color: "transparent"

    property var gridView: inGrid ? parent.GridView.view : null
    readonly property bool inGrid : null != model ? true : false

    property real margin: 4
    width: inGrid ? gridView.cellWidth : 64
    height: inGrid ? gridView.cellHeight : 64

    property SaveData dto: model.saveData
    property Resource resource: model.resource
    property bool showText: true
    property int value: model.saveValue ? model.saveValue : 0

    IconResourceDelegate {
        id: resourceIcon
        anchors.centerIn: parent
        width: parent.width - 2*parent.margin
        height: parent.width - 2*parent.margin
        pResource: achievementItem.resource

        // Gloss
        Rectangle {
            id: glossRect
            x: (resourceIcon.width - resourceIcon.width) / 2
            y: (resourceIcon.height - resourceIcon.width) / 2
            width: resourceIcon.paintedWidth
            height: resourceIcon.paintedHeight
            color: "transparent"

            LinearGradient {
                anchors.fill: parent
                start: Qt.point(parent.width, 0)
                end: Qt.point(parent.width * 2 / 3, parent.height / 2)
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: achievementItem.isSelected ? "#DDF0F0F0" : "#AAF0F0F0"
                    }
                    GradientStop {
                        position: 1.0
                        color: "#00F0F0F0"
                    }
                }
            }
        }
    }

    Desaturate {
        id: desaturateEffect
        anchors.fill: resourceIcon
        source: resourceIcon
        desaturation: progressRect.progress < progressRect.progressMax ? 1.0 : 0.0
    }

    // progressBar
    TimerDisplay {
        id: progressDisplay
        anchors.fill: parent
        fillColor: "transparent"

        smooth: Settings.playerImageSmooth
        antialiasing: Settings.playerAntialiasing

        primaryColor: root.style ? root.style.timerDisplay.primaryColor : "transparent"
        secondaryColor: root.style ? root.style.timerDisplay.secondaryColor : "transparent"
        tertiaryColor: root.style ? root.style.timerDisplay.tertiaryColor : "transparent"

        borderWidth: root.style ? root.style.timerDisplay.borderWidth : 0
        groveWidth: root.style ? root.style.timerDisplay.groveWidth : 0

        timeMsMax: progressRect.progressMax
        timeMsCurrent: progressRect.progressMax - progressRect.progress
        updateCounter: 0
        visibleCounter: true
    }

    // Progress Background
    Rectangle {
        id: progressRect
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        height: 10
        width: parent.width
        color: "#88000000"

        visible: achievementItem.showText

        property int progress: achievementItem.value
        property int progressMax: {
            if (null == dto) return 0;
            switch (dto.type)
            {
                case SaveData.Bool:
                    return 1;
                case SaveData.Int:
                    return dto.data;
                default: return -1;
            }
        }
    }

    Label {
        id: text
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        height: 14
        color: "white"

        horizontalAlignment: Text.AlignHCenter

        text: "" + progressRect.progress + "/" + progressRect.progressMax

        visible: achievementItem.showText
    }
}
