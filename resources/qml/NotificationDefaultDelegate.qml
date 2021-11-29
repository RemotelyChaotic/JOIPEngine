import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1
import JOIP.script 1.1

Rectangle {
    id: defaultDelegate
    width: parent.ListView.view.width
    height: notificationBackground.height + 40
    color: "transparent"

    Rectangle {
        id: notificationBackground
        anchors.verticalCenter: parent.verticalCenter
        x: {
            switch (model.iconAlignment)
            {
                case TextAlignment.AlignCenter: return defaultDelegate.width / 2 - width / 2;
                case TextAlignment.AlignLeft: return 20; // a little bit of a margin looks better
                case TextAlignment.AlignRight: return defaultDelegate.width - width - 20;
            }
            return defaultDelegate.width / 2 - width / 2;
        }
        width: layoutText.width + 30
        height: layoutText.height + 20
        color: model.backgroundColor
        radius: 5

        ColumnLayout {
            id: layoutText
            anchors.centerIn: parent
            width: Math.max(text.contentWidth, buttonText.contentWidth + 20)
            height: text.contentHeight + spacing + buttonText.contentHeight + 20

            Text {
                id: text
                Layout.alignment: Qt.AlignHCenter

                font.family: root.currentlyLoadedProject.font
                font.pointSize: 14
                font.hintingPreference: Font.PreferNoHinting
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideNone
                text: model.title.replace("<html>","").replace("</html>","")
                wrapMode: Text.WordWrap
                color: model.textColor
                textFormat: (model.title.startsWith("<html>") && model.title.endsWith("</html>")) ?
                                Text.RichText :
                                (QtApp.mightBeRichtext(model.title) ?
                                     Text.StyledText : Text.PlainText)

                visible: model.title !== ""
            }

            Button {
                id: button

                Layout.alignment: Qt.AlignHCenter
                text: model.buttonText;

                visible: model.buttonText !== ""

                onHoveredChanged: {
                    if (hovered)
                    {
                        root.soundEffects.hoverSound.play();
                    }
                }
                onClicked: {
                    root.soundEffects.clickSound.play();
                    defaultDelegate.parent.ListView.view.buttonPressed(model.sId, model.sOnButton);
                }

                background: Rectangle {
                    anchors.fill: parent
                    color: model.backgroundColorWidget
                    radius: 5
                }

                contentItem: Text {
                    id: buttonText
                    anchors.centerIn: parent
                    font.family: root.currentlyLoadedProject.font;
                    font.pointSize: text.pointSize
                    font.hintingPreference: Font.PreferNoHinting
                    elide: Text.ElideNone
                    text: button.text
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: model.textColorWidget
                    textFormat: QtApp.mightBeRichtext(text) ? Text.RichText : Text.PlainText
                }

                Shortcut {
                    sequence: Settings.keyBinding("Right_" + (index+1));
                    onActivated: {
                        root.soundEffects.clickSound.play();
                        defaultDelegate.parent.ListView.view.buttonPressed(model.sId, model.sOnButton);
                    }
                }
            }
        }

        TimedItem {
            id: timer
            x: {
                switch (model.iconAlignment)
                {
                    case TextAlignment.AlignCenter: return parent.width / 2 - width / 2;
                    case TextAlignment.AlignLeft: return parent.width - width / 5;
                    case TextAlignment.AlignRight: return -width * 4 / 5;
                }
                return parent.width / 2 - width / 2;
            }
            y: {
                switch (model.iconAlignment)
                {
                    case TextAlignment.AlignCenter: return -height * 3 / 4;
                    case TextAlignment.AlignLeft: return parent.height / 2 - height / 2;
                    case TextAlignment.AlignRight: return parent.height / 2 - height / 2;
                }
                return -height * 3 / 4;
            }
            width: (pResource === null || model.timeMs > 0) ? 0 : 64
            height: 64

            showTime: true
            showTimeNumber: false
            maxTimeMs: model.timeMs

            property Resource pResource: model.portrait

            background: IconResourceDelegate {
                id: portraitBg
                x: parent.x
                y: parent.y
                width: parent.widgth
                height: parent.height

                pResource: parent.pResource
            }

            onTimeout: {
                defaultDelegate.parent.ListView.view.timeout(model.sId, model.sOnTimeout);
            }
        }
    }

    Component.onCompleted: {
        if (model.timeMs > 0) {
            timer.start();
        }
    }
}
