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


            PlayerIcons {
                id: icon
                Layout.preferredWidth: (parent.width - parent.spacing * 2) / 4
                Layout.preferredHeight: parent.height
                Layout.alignment: Qt.AlignVCenter
                userName: "icon"
            }

            PlayerMediaPlayer {
                id: mediaPlayer
                Layout.preferredWidth: (parent.width - parent.spacing * 2) / 2
                Layout.preferredHeight: parent.height
                Layout.alignment: Qt.AlignVCenter
                userName: "mediaPlayer"
            }

            PlayerTimer {
                id: timer
                Layout.preferredWidth: (parent.width - parent.spacing * 2) / 4
                Layout.preferredHeight: parent.height
                Layout.alignment: Qt.AlignVCenter
                userName: "timer"
            }
        }

        PlayerTextBox {
            id: textBox;
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height / 3 - parent.spacing / 2
            Layout.alignment: Qt.AlignHCenter
            userName: "textBox"
        }
    }
}
