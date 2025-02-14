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

    property int value: 0
    property SaveData save: null
    property Resource resource: null

    Rectangle {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - parent.height/2
        height: parent.height

        color: root.style ? root.style.achievementDisplay.backgroundColor : "transparent"
        radius: root.style ? root.style.achievementDisplay.borderRadius : 0
        border.color: root.style ? root.style.achievementDisplay.borderColor : "transparent"
        border.width: root.style ? root.style.achievementDisplay.borderWidth : 1
    }
    Rectangle {
        anchors.left: parent.left
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
        x: 5
        anchors.verticalCenter: parent.verticalCenter
        width: parent.height-10
        height: parent.height-10

        dto: achievement.save
        resource: achievement.resource
        showText: false
        value: achievement.value
    }
    TextItemFormated {
        id: textContentItem
        x: parent.height + 5
        anchors.verticalCenter: parent.verticalCenter

        maximumWidth: parent.width - parent.height - 10

        text: null != save ? save.name : ""
        textColor: root.style ? root.style.achievementDisplay.textColor : "white"
    }
}
