import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1
import JOIP.script 1.1

Rectangle {
    id: textDelegate
    width: parent.ListView.view.width
    height: textBackground.height + 40
    color: "transparent"

    smooth: Settings.playerImageSmooth
    antialiasing: Settings.playerAntialiasing

    property int iconWidth: parent.ListView.view.iconWidth
    property int iconHeight: parent.ListView.view.iconHeight

    Rectangle {
        id: textBackground
        anchors.verticalCenter: parent.verticalCenter
        x: {
            switch (textAlignment)
            {
                case TextAlignment.AlignCenter: return textDelegate.width / 2 - width / 2;
                case TextAlignment.AlignLeft: return 20; // a little bit of a margin looks better
                case TextAlignment.AlignRight: return textDelegate.width - width - 20;
            }
            return textDelegate.width / 2 - width / 2;
        }
        width: textContentItem.width + imageSkip.width + 30
        height: textContentItem.height + 20
        color: backgroundColor
        radius: 5

        TextItemFormated {
            id: textContentItem
            anchors.centerIn: parent

            maximumWidth: textDelegate.width - 50 - 24 // 24 is imageSkip.width at it's max
                                                       // 50 is for spacing

            text: textContent
            textColor: model.textColor
        }

        IconResourceDelegate {
            id: portrait
            x: {
                switch (textAlignment)
                {
                    case TextAlignment.AlignCenter: return parent.width / 2 - width / 2;
                    case TextAlignment.AlignLeft: return parent.width - width / 5;
                    case TextAlignment.AlignRight: return -width * 4 / 5;
                }
                return parent.width / 2 - width / 2;
            }
            y: {
                switch (textAlignment)
                {
                    case TextAlignment.AlignCenter: return -height * 3 / 4;
                    case TextAlignment.AlignLeft: return parent.height / 2 - height / 2;
                    case TextAlignment.AlignRight: return parent.height / 2 - height / 2;
                }
                return -height * 3 / 4;
            }
            width: pResource === null ? 0 : textDelegate.iconWidth
            height: textDelegate.iconHeight

            pResource: model.portrait
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
