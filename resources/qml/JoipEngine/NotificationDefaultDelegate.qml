import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.2
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: defaultDelegate
    width: parent.ListView.view.width
    height: Math.min(timer.height, notificationBackground.height) + 15
    color: "transparent"

    Rectangle {
        id: notificationBackground
        anchors.centerIn: parent
        width: layoutText.width + 10
        height: layoutText.height + 10
        color: model.backgroundColor
        radius: 5

        ColumnLayout {
            id: layoutText
            anchors.centerIn: parent
            width: Math.max(text.width, buttonText.contentWidth + 10)
            height: text.visibleHeight +
                    ((text.visible && button.visible) ? spacing : 0) +
                    button.visibleHeight

            TextItemFormated {
                id: text

                property real visibleHeight: visible ? height : 0

                Layout.preferredHeight: visibleHeight
                Layout.alignment: Qt.AlignHCenter

                maximumWidth: defaultDelegate.width

                text: model.title
                textColor: model.textColor

                visible: model.title !== ""
            }

            Button {
                id: button

                property real visibleHeight: visible ? (buttonText.contentHeight + 10) : 0

                Layout.preferredWidth: buttonText.contentWidth + 10
                Layout.preferredHeight: visibleHeight
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
                    font.pointSize: 14
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
    }

    TimedItem {
        id: timer
        x: {
            switch (model.iconAlignment)
            {
                default:
                case TextAlignment.AlignLeft: return defaultDelegate.width / 2 - width + 5 - notificationBackground.width / 2;
                case TextAlignment.AlignRight: return defaultDelegate.width / 2 + width - 5 + - notificationBackground.width / 2;
            }
        }
        y: {
            switch (model.iconAlignment)
            {
                default:
                case TextAlignment.AlignLeft: return defaultDelegate.height / 2 - height / 2;
                case TextAlignment.AlignRight: return defaultDelegate.height / 2 - height / 2;
            }
        }
        width: (pResource !== null || model.timeMs > 0) ? 48 : 0
        height: 48

        showClock: maxTimeMs > 0
        showTime: true
        showTimeNumber: false
        maxTimeMs: model.timeMs

        property Resource pResource: model.portrait

        background: IconResourceDelegate {
            id: portraitBg
            anchors.fill: parent
            pResource: timer.pResource
        }

        onTimeout: {
            defaultDelegate.parent.ListView.view.timeout(model.sId, model.sOnTimeout);
        }
    }

    Component.onCompleted: {
        if (model.timeMs > 0) {
            timer.timeMs = model.timeMs;
            timer.start();
        }

        NotificationManager.registerId(sId);
    }

    // handle interrupt
    Connections {
        target: ScriptRunner
        onRunningChanged: {
            if (timer.bStarted) {
                if (!bRunning)
                {
                    timer.pause();
                }
                else
                {
                    timer.resume();
                }
            }
        }
    }
}
