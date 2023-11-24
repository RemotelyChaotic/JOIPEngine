import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1
import JOIP.script 1.1

Rectangle {
    id: textDelegate

    property var listView: inList ? parent.ListView.view : null
    readonly property bool inList : null != model ? true : false

    width: inList ? listView.width : textBox.width
    height: textBackground.height + 40
    color: "transparent"

    // asign model or parent properties for reference in delegate
    readonly property int iconWidthLocal: inList ? listView.iconWidth : iconWidth;
    readonly property int iconHeightLocal: inList ? listView.iconHeight : iconHeight
    property var backgroundColor: inList ? model.backgroundColor : "black"
    property var textColor: inList ? model.textColor : "white"
    property string textContent: inList ? model.textContent : ""
    property int textAlignment: inList ? model.textAlignment : 0
    property var portrait: inList ? model.portrait : null

    smooth: Settings.playerImageSmooth
    antialiasing: Settings.playerAntialiasing

    Rectangle {
        id: textBackground
        anchors.verticalCenter: parent.verticalCenter
        x: {
            switch (textDelegate.textAlignment)
            {
                case TextAlignment.AlignCenter: return textDelegate.width / 2 - width / 2;
                case TextAlignment.AlignLeft: return 20; // a little bit of a margin looks better
                case TextAlignment.AlignRight: return textDelegate.width - width - 20;
            }
            return textDelegate.width / 2 - width / 2;
        }
        width: textContentItem.width + imageSkip.width + 30
        height: textContentItem.height + 20
        color: textDelegate.backgroundColor
        radius: 5

        TextItemFormated {
            id: textContentItem
            anchors.centerIn: parent

            maximumWidth: textDelegate.width - 50 - 24 // 24 is imageSkip.width at it's max
                                                       // 50 is for spacing

            text: textDelegate.textContent
            textColor: textDelegate.textColor
        }

        IconResourceDelegate {
            id: portraitDelegate
            x: {
                switch (textDelegate.textAlignment)
                {
                    case TextAlignment.AlignCenter: return parent.width / 2 - width / 2;
                    case TextAlignment.AlignLeft: return parent.width - width / 5;
                    case TextAlignment.AlignRight: return -width * 4 / 5;
                }
                return parent.width / 2 - width / 2;
            }
            y: {
                switch (textDelegate.textAlignment)
                {
                    case TextAlignment.AlignCenter: return -height * 3 / 4;
                    case TextAlignment.AlignLeft: return parent.height / 2 - height / 2;
                    case TextAlignment.AlignRight: return parent.height / 2 - height / 2;
                }
                return -height * 3 / 4;
            }
            width: pResource === null ? 0 : textDelegate.iconWidthLocal
            height: textDelegate.iconHeightLocal

            pResource: textDelegate.portrait
        }

        Image {
            id: imageSkip
            x: -20
            height: 24
            width: bShow ? 24 : 0
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            source: Settings.styleFolderQml() + "/ButtonPlay.svg";

            property bool bIsLast: inList ? (index+1 < listView.count ? false : true) : true
            property bool bShow: bIsLast && (inList ? listView.skippable : skippable)

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

    opacity: 0
    OpacityAnimator on opacity{
        from: 0;
        to: 1;
        duration: 500
    }

    Component.onCompleted: {
        if (inList) {
            listView.delegateComponentLoaded();
        } else {
            delegateComponentLoaded();
        }
    }
}
