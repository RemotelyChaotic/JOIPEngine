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
        width: layoutText.width + imageSkip.width + 30
        height: layoutText.height + 20
        color: backgroundColor
        radius: 5

        Rectangle {
            id: layoutText
            anchors.centerIn: parent
            width: text.implicitWidth
            height: text.implicitHeight
            color: "transparent"

            Text {
                id: text
                anchors.centerIn: parent
                font.family: Settings.font;
                font.pointSize: 14
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideNone
                text: textContent
                wrapMode: Text.WordWrap
                color: textColor
                textFormat: QtApp.mightBeRichtext(textContent) ? Text.RichText : Text.PlainText
            }
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
            width: pResource === null ? 0 : 64
            height: 64

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
