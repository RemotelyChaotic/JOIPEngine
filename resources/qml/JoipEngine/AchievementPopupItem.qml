import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.5
import JOIP.db 1.5
import JOIP.ui 1.1

Rectangle {
    id: achievement
    color: "transparent"

    property int orientation: Settings.dominantHand
    property int value: 0
    property real openState: 1.0
    property SaveData save: null
    property Resource resource: null

    Rectangle {
        id: bgBox
        x: achievement.orientation === DominantHand.Right ?
               parent.height/2 :
               (parent.width - parent.height/2) * (1-achievement.openState)
        anchors.verticalCenter: parent.verticalCenter
        width: (parent.width - parent.height/2) * achievement.openState
        height: parent.height

        color: root.style ? root.style.achievementDisplay.backgroundColor : "transparent"
        radius: root.style ? root.style.achievementDisplay.borderRadius : 0
        border.color: root.style ? root.style.achievementDisplay.borderColor : "transparent"
        border.width: root.style ? root.style.achievementDisplay.borderWidth : 1
    }
    Rectangle {
        x: achievement.orientation === DominantHand.Right ? 0 : parent.width - parent.height
        anchors.verticalCenter: parent.verticalCenter
        width: parent.height
        height: parent.height

        color: root.style ? root.style.achievementDisplay.backgroundColor : "transparent"
        radius: parent.height/2
        border.color: root.style ? root.style.achievementDisplay.borderColor : "transparent"
        border.width: root.style ? root.style.achievementDisplay.borderWidth : 0
    }
    AchievementItemDelegate {
        id: achDelegate
        x: achievement.orientation === DominantHand.Right ? 5 : parent.width - 5
        anchors.verticalCenter: parent.verticalCenter
        width: parent.height-10
        height: parent.height-10

        dto: achievement.save
        resource: achievement.resource
        showProgress: showText
        showText: null != save ? (save.data == achievement.value ? false : true) : false
        value: achievement.value
        bgVisible: false

        layer.enabled: true
        layer.effect: Glow {
            radius: 8
            samples: 17
            spread: 0.5
            color: root.style ? root.style.achievementDisplay.glowColor : "white"
            transparentBorder: false
        }
    }
    TextItemFormated {
        id: textContentItem
        x: achievement.orientation === DominantHand.Right ? parent.height + 5 : bgBox.x + 5
        anchors.verticalCenter: parent.verticalCenter

        maximumWidth: bgBox.width - 10 - parent.height/2

        text: null != save ? save.name : ""
        textColor: root.style ? root.style.achievementDisplay.textColor : "white"
    }
}
