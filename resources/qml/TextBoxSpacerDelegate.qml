import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1

Rectangle {
    id: textDelegate
    width: parent.ListView.view.width
    height: parent.ListView.view.height
    color: "transparent"

    Component.onCompleted: {
        textDelegate.parent.ListView.view.delegateComponentLoaded();
    }
}
