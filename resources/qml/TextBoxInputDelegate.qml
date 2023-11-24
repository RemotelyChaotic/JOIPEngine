import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1

Rectangle {
    id: textDelegate

    property var listView: inList ? parent.ListView.view : null
    readonly property bool inList : null != model ? true : false

    width: inList ? listView.width : textBox.width
    height: 40 + 40
    color: "transparent"

    // asign model or parent properties for reference in delegate
    property var backgroundColor: inList ? model.backgroundColor : "black"
    property var textColor: inList ? model.textColor : "white"
    property string textContent: inList ? model.textContent : ""
    property string requestId: inList ? model.sRequestId : ""
    property bool storeIntoStorageInstead: inList ? model.storeIntoStorageInstead : false

    smooth: Settings.playerImageSmooth
    antialiasing: Settings.playerAntialiasing

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
        color: textDelegate.backgroundColor
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
            color: textDelegate.textColor

            selectByMouse: true

            property string placeholderText: "....."
            property bool displayPlaceHolder: !text && !focus
            property string storeIntoVar: textDelegate.textContent
            property bool storeVarIntoStorageInstead: textDelegate.storeIntoStorageInstead

            Text {
                id: overlay
                anchors.horizontalCenter: parent.horizontalCenter
                text: textInput.placeholderText
                color: textDelegate.textColor
                visible: textInput.displayPlaceHolder
            }

            onEditingFinished: {
                textInput.focus = false;
                textInput.cursorVisible = false;
                if (inList) {
                    listView.inputEditingFinished(textInput.text, textInput.storeIntoVar,
                                                  textDelegate.requestId, textInput.storeVarIntoStorageInstead);
                } else {
                    inputEditingFinished(textInput.text, textInput.storeIntoVar,
                                         textDelegate.requestId, textInput.storeVarIntoStorageInstead);
                }
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
        if (inList) {
            listView.delegateComponentLoaded();
        } else {
            delegateComponentLoaded();
        }
    }
}
