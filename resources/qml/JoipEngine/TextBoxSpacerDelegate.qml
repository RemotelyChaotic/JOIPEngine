import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.5

Rectangle {
    id: textDelegate
    width: parent.ListView.view.width
    height: parent.ListView.view.height
    color: "transparent"

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.skipWait();
        }
    }

    Component.onCompleted: {
        textDelegate.parent.ListView.view.delegateComponentLoaded();
    }
}
