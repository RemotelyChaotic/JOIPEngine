import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1

Rectangle {
    id: textDelegate
    width: parent.ListView.view.width
    height: textBackground.height + 40
    color: "transparent"

    Rectangle {
        id: textBackground
        anchors.centerIn: parent
        width: text.width + 20
        height: text.height + 20
        color: backgroundColor
        radius: 5

        Text {
            id: text
            anchors.centerIn: parent
            font.family: Settings.font;
            font.pointSize: 14
            elide: Text.ElideNone
            text: textContent
            wrapMode: Text.WordWrap
            color: textColor
        }
    }

    Component.onCompleted: {
        textDelegate.parent.ListView.view.delegateComponentLoaded();
    }
}
