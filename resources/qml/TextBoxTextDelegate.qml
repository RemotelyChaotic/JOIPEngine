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
        width: layoutText.width + imageSkip.width + 30
        height: layoutText.height + 20
        color: backgroundColor
        radius: 5

        Rectangle {
            id: layoutText
            anchors.centerIn: parent
            width: text.contentWidth
            height: text.contentHeight
            color: "transparent"

            Text {
                id: text
                anchors.centerIn: parent
                font.family: Settings.font;
                font.pointSize: 14
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideNone
                text: textContent
                wrapMode: Text.Wrap
                color: textColor
            }
        }

        Image {
            id: imageSkip
            x: -20
            height: 24
            width: bShow ? 24 : 0
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            source: Settings.styleFolderQml() + "/ButtonPlay.svg";

            property bool bIsLast: index+1 < textDelegate.parent.ListView.view.count ? false : true
            property bool bShow: bIsLast && textDelegate.parent.ListView.view.skippable

            opacity: bShow ? 1.0 : 0.0
            Behavior on opacity {
                NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
            }
        }
    }

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