import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1

Rectangle {
    id: textDelegate
    width: parent.ListView.view.width
    height: 40 + 40
    color: "transparent"

    property var requestId: sRequestId

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.skipWait();
        }
    }

    Rectangle {
        id: textBackground
        anchors.centerIn: parent
        width: textInput.width + 20
        height: 40
        color: backgroundColor
        radius: 5

        TextInput {
            id: textInput
            width: textMetrics.boundingRect.width < 200 ? 200 : textMetrics.boundingRect.width
            anchors.centerIn: parent

            font.family: root.currentlyLoadedProject.font;
            font.pointSize: 14
            font.hintingPreference: Font.PreferNoHinting
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            color: textColor

            selectByMouse: true

            property string placeholderText: "....."
            property bool displayPlaceHolder: !text && !focus
            property string storeIntoVar: textContent
            property bool storeVarIntoStorageInstead: storeIntoStorageInstead

            Text {
                id: overlay
                anchors.horizontalCenter: parent.horizontalCenter
                text: textInput.placeholderText
                color: textColor
                visible: textInput.displayPlaceHolder
            }

            onEditingFinished: {
                textInput.focus = false;
                textInput.cursorVisible = false;
                textDelegate.parent.ListView.view.inputEditingFinished(textInput.text, textInput.storeIntoVar,
                                                                       textDelegate.requestId, textInput.storeVarIntoStorageInstead);
                textInput.readOnly = true;
            }
        }
    }

    TextMetrics {
        id: textMetrics
        font.family: textInput.font.family
        font.pointSize: textInput.font.pointSize
        text: textInput.text
    }

    Component.onCompleted: {
        textDelegate.parent.ListView.view.delegateComponentLoaded();
    }
}
