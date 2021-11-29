import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14

Rectangle {
    id: layout
    anchors.fill: parent
    color: "transparent"

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        RowLayout {
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height * 2 / 3 - parent.spacing / 2
            Layout.alignment: Qt.AlignHCenter
            spacing: 5


            Rectangle {
                Layout.preferredWidth: (parent.width - parent.spacing * 2) / 4
                Layout.preferredHeight: parent.height
                Layout.alignment: Qt.AlignVCenter
                color: "transparent"

                PlayerIcons {
                    id: icon
                    width: parent.width - 50
                    height: parent.height - 50
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    userName: "icon"
                }
            }

            PlayerMediaPlayer {
                id: mediaPlayer
                Layout.preferredWidth: (parent.width - parent.spacing * 2) / 2
                Layout.preferredHeight: parent.height
                Layout.alignment: Qt.AlignVCenter
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
                Layout.preferredWidth: (parent.width - parent.spacing * 2) / 4
                Layout.preferredHeight: parent.height
                Layout.alignment: Qt.AlignVCenter
                color: "transparent"

                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    width: parent.Layout.preferredWidth
                    height: parent.Layout.preferredHeight / 2
                    color: "transparent"

                    PlayerTimer {
                        id: timer
                        anchors.centerIn: parent
                        width: 138
                        height: 138
                        userName: "timer"
                    }
                }
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    width: parent.Layout.preferredWidth
                    height: parent.Layout.preferredHeight / 2
                    color: "transparent"

                    PlayerNotification {
                        id: notification
                        anchors.fill: parent
                        userName: "notification"
                    }
                }
            }
        }

        PlayerTextBox {
            id: textBox;
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height / 3 - parent.spacing / 2
            Layout.alignment: Qt.AlignHCenter
            userName: "textBox"
            mainTextBox: true
        }
    }
}
