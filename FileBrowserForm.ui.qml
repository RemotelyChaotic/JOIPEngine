import QtQuick 2.4
import QtQuick.Controls 2.2

Item {
    id: item1
    width: 500
    height: 50
    property alias pathTextField: pathTextField
    property alias browseButton: browseButton

    Button {
        id: browseButton
        width: 50
        height: 50
        text: qsTr("...")
        anchors.right: parent.right
        anchors.rightMargin: 0
    }

    TextField {
        id: pathTextField
        width: parent.width - 50
        height: 50
        text: Settings.contentFolder
        font.pixelSize: Qt.application.font.pixelSize
        anchors.left: parent.left
        anchors.leftMargin: 0
    }
}
