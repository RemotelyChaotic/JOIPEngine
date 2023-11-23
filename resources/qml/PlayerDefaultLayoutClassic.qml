import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1

Rectangle {
    id: layout
    anchors.fill: parent
    color: "transparent"

    property int spacing: 5

    readonly property bool isMobile: Settings.platform === "Android"
    readonly property int dominantHand: Settings.dominantHand
    readonly property int iconWidth: isMobile ? 32 : 64
    readonly property int iconHeight: isMobile ? 32 : 64
    property bool isLandscape: { return width > height; }

    PlayerMediaPlayer {
        id: mediaPlayer

        anchors.top: parent.top
        x: !isMobile ? (parent.width - width) / 2 :
                       (DominantHand.Left === dominantHand && isLandscape ? parent.width / 2 :
                                                                            0)

        width: !isMobile ? ((parent.width - parent.spacing * 2) / 2) :
                           (isLandscape ? parent.width / 2 :
                                          parent.width)
        height: !isMobile ? (parent.height * 2 / 3 - parent.spacing / 2) :
                            (isLandscape ? parent.height :
                                           (parent.height * 2 / 3 - parent.spacing / 2))
        userName: "mediaPlayer"
        mainMediaPlayer: true

        PlayerMetronome {
            id: metronome
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            height: parent.height / 6
            userName: "metronome"
        }
    }

    Rectangle {
        id: iconRect
        anchors.top: parent.top
        x: !isMobile ? 0 :
                       (isLandscape ? (DominantHand.Left === dominantHand ? 0 :
                                                                            mediaPlayer.x + mediaPlayer.width + parent.spacing) :
                                       0)

        width: (parent.width - parent.spacing * 2) / 4
        height: !isMobile ? mediaPlayer.height :
                            (isLandscape ? ((parent.height - parent.spacing) / 2) :
                                           mediaPlayer.height)
        color: "transparent"

        PlayerIcons {
            id: icon
            width: parent.width - layout.spacing
            height: parent.height - layout.spacing
            iconWidth: layout.iconWidth
            iconHeight: layout.iconHeight

            anchors.right: parent.right
            anchors.bottom: parent.bottom
            userName: "icon"
        }
    }

    Rectangle {
        id: timerRect
        anchors.top: parent.top
        x: !isMobile || DominantHand.Left !== dominantHand ? parent.width - width :
                                                             (isLandscape ? mediaPlayer.x - width :
                                                                            parent.width - width)

        width: (parent.width - parent.spacing * 2) / 4
        height: !isMobile ? mediaPlayer.height :
                            (isLandscape ? ((parent.height - parent.spacing) / 2) :
                                           mediaPlayer.height)
        color: "transparent"

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            width: parent.width
            height: !isMobile ? parent.height / 2 : width
            color: "transparent"

            PlayerTimer {
                id: timer
                anchors.centerIn: parent
                width: Math.min(138, parent.width)
                height: Math.min(138, parent.width)
                userName: "timer"
            }
        }
    }

    Rectangle {
        anchors.bottom: isMobile && isLandscape ? iconRect.bottom :
                                                  timerRect.bottom
        anchors.left: isMobile && isLandscape ? iconRect.left :
                                                (DominantHand.Left === dominantHand ? parent.left :
                                                                                      timerRect.left)
        width: isMobile && isLandscape ? iconRect.width : timerRect.width
        height: isMobile && isLandscape ? iconRect.height * 2 / 3 : timerRect.height / 2
        color: "transparent"

        PlayerNotification {
            id: notification
            anchors.fill: parent
            userName: "notification"
        }
    }

    PlayerTextBox {
        id: textBox;

        anchors.bottom: parent.bottom
        x: !isMobile ? 0 :
                       (isLandscape && DominantHand.Left !== dominantHand ? (parent.width + parent.spacing) / 2 :
                                                                            0)

        width: !isMobile ? parent.width :
                           (isLandscape ? (parent.width - parent.spacing) / 2 : parent.width)
        height: !isMobile ? (parent.height / 3 - parent.spacing / 2) :
                            (isLandscape ? ((parent.height - parent.spacing) / 2) :
                                           (parent.height / 3 - parent.spacing / 2))
        iconWidth: layout.iconWidth
        iconHeight: layout.iconHeight

        userName: "textBox"
        mainTextBox: true
    }

    PlayerControls {
        id: sceneControl

        x: textBox.x
        y: textBox.y - height / 2
        width: textBox.width - parent.spacing
        height: 32

        buttonHeight: 32
        buttonWidth: 48
        spacing: parent.spacing
        soundEffects: root.soundEffects
    }
}
