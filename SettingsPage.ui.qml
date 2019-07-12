import QtQuick 2.9
import QtQuick.Controls 2.2

Page {
    title: qsTr("Settings")

    Grid {
        id: grid
        layoutDirection: Qt.RightToLeft
        flow: Grid.TopToBottom
        spacing: 10
        rows: 1
        columns: 2
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        anchors.fill: parent

        Item {
            id: fileOption
            width: parent.width - 90
            height: 50

            FileBrowser {
                id: fileBrowserForm
                x: 0
                y: 0
                width: parent.width
            }
        }
        Text {
            id: label1
            width: 80
            height: 50
            text: qsTr("Data Path")
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: Qt.application.font.pixelSize * 1.6
        }
    }
}


/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
