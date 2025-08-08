import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import JOIP.core 1.5

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

        anchors.fill: parent
        userName: "mediaPlayer"
        mainMediaPlayer: true

        PlayerMetronome {
            id: metronome
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            height: Math.max(parent.height * Settings.metronomeSizeRel, Settings.metronomeSizeMin)
            userName: "metronome"
        }
    }

    Rectangle {
        id: iconRect
        anchors.top: parent.top
        x: DominantHand.Left === dominantHand ? 0 : parent.width - width

        width: (parent.width - parent.spacing * 2) / 4
        height: parent.height - textBox.height;
        color: "transparent"

        PlayerIcons {
            id: icon
            width: parent.width
            height: parent.height
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
        x: DominantHand.Left === dominantHand ? parent.width - width : 0

        width: (parent.width - parent.spacing * 2) / 4
        height: parent.height - textBox.height;
        color: "transparent"

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            width: !isMobile ? parent.width : parent.width
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
        id: notificationRect

        anchors.bottom: textBox.top
        x: DominantHand.Left === dominantHand ? 0 : parent.width - width

        width: (parent.width - parent.spacing * 2) / 4
        height: (parent.height - textBox.height - layout.spacing) / 2
        color: "transparent"

        PlayerNotification {
            id: notification
            anchors.fill: parent
            userName: "notification"
        }
    }

    PlayerTextBox {
        id: textBox;

        anchors.left: parent.left
        anchors.bottom: parent.bottom

        width: parent.width
        height: parent.height / 3 - parent.spacing / 2
        iconWidth: layout.iconWidth
        iconHeight: layout.iconHeight

        userName: "textBox"
        mainTextBox: true
        displayMode: PlayerTextBox.TextBoxMode.TextBox
        hideLogAfterInactivity: true
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

        onSetUiVisible: {
            iconRect.opacity = visible ? 1.0 : 0.0;
            timerRect.opacity = visible ? 1.0 : 0.0;
            notificationRect.opacity = visible ? 1.0 : 0.0;
            textBox.opacity = visible ? 1.0 : 0.0;
        }

        Button {
            id: showLogButton
            height: sceneControl.height
            Layout.preferredWidth: 48
            Layout.alignment: Qt.AlignVCenter

            focusPolicy: Qt.NoFocus
            enabled: textBox.opacity > 0

            text: Settings.keyBinding("Tools")
            contentItem: Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
                rightPadding: 5
                bottomPadding: 5

                text: parent.text
                font.family: Settings.font
                font.pixelSize: 8
                color: "white"
            }

            ToolTip.delay: 1000
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Show or hide log") + " (" + Settings.keyBinding("Tools") + ")"

            Image {
                anchors.centerIn: parent
                width: (parent.width < parent.height ? parent.width : parent.height) - 10
                height: (parent.width < parent.height ? parent.width : parent.height) - 10
                fillMode: Image.PreserveAspectFit
                source: Settings.styleFolderQml() + (textBox.logShown ?
                            "/ButtonArrowDown.svg" : "/ButtonArrowUp.svg")
            }

            onHoveredChanged: {
                if (hovered)
                {
                    soundEffects.hoverSound.play();
                }
            }
            onClicked: {
                textBox.logShown = !textBox.logShown
            }

            Shortcut {
                sequence: Settings.keyBinding("Tools")
                context: Qt.ApplicationShortcut
                onActivated: {
                    textBox.logShown = !textBox.logShown
                }
            }
        }
    }
}
