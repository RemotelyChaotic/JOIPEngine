import QtQuick 2.9
import QtQuick.Controls 2.2

Page {
    title: qsTr("Home")

    Grid {
        id: grid
        layoutDirection: Qt.LeftToRight
        flow: Grid.TopToBottom
        spacing: 10
        rows: 2
        columns: 1
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        anchors.fill: parent

        Item {
            id: buttonBar
            width: parent.width
            height: 50

            Button {
                id: button
                x: parent.width / 2 - width / 2
                y: 0
                text: qsTr("Button")
                transformOrigin: Item.Center
            }
        }

        ScrollView {
            id: scrollView
            width: parent.width
            height: parent.height - buttonBar.height
            y: buttonBar.height
        }
    }
}


/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:30;anchors_width:200}D{i:29;anchors_height:400;anchors_width:400;anchors_x:120;anchors_y:40}
}
 ##^##*/
