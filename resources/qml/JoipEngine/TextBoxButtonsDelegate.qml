import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1

Rectangle {
    id: textDelegate

    property var listView: inList ? parent.ListView.view : null
    readonly property bool inList : null != model ? true : false

    width: inList ? listView.width : textBox.width
    height: tableView.itemHeight * (tableView.rowCount+1) + tableView.spacing * tableView.rowCount + 40
    color: "transparent"

    property var backgroundColors: inList ? JSON.parse(model.backgroundColors) : []
    property var textColors: inList ? JSON.parse(model.textColors) : []
    property var buttonTexts: inList ? model.buttonTexts : []
    property string textContent: inList ? model.textContent : ""
    property string requestId: inList ? model.sRequestId : ""
    property bool storeIntoStorageInstead: inList ? model.storeIntoStorageInstead : false

    TextMetrics {
        id: textMetrics
        font.family: root.currentlyLoadedProject.font
        font.pointSize: 14
        text: "testText here"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.skipWait();
        }
    }

    Rectangle {
        id: tableView
        color: "transparent"
        x: 20
        width: parent.width - 40
        height: itemHeight * (rowCount+1) + spacing * rowCount

        property int spacing: 20
        property int itemHeight: textMetrics.boundingRect.height + 10
        property int rowCount: repeater.rowCount

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }

        Repeater {
            id: repeater
            model: buttonTexts

            property int currentRowWidth: 0
            property int numAddedItems: 0
            property int rowCount: 0
            property var rowSizes: []

            // we need to calculate positions manually
            onItemAdded: {
                numAddedItems++;
                item.x = currentRowWidth;
                if (currentRowWidth + item.width + tableView.spacing > tableView.width)
                {
                    rowSizes[rowCount] = currentRowWidth - tableView.spacing;
                    rowCount++;
                    item.x = 0;
                    currentRowWidth = item.width + tableView.spacing;
                }
                else
                {
                    currentRowWidth += item.width + tableView.spacing;
                }
                item.y = rowCount * tableView.itemHeight + rowCount * tableView.spacing;
                item.rowIndex = rowCount;
                rowSizes[rowCount] = currentRowWidth - tableView.spacing;

                // all items added
                if (numAddedItems === model.length)
                {
                    for (var i = 0; i < model.length; ++i)
                    {
                        var pItem = itemAt(i);
                        var iRowWidth = rowSizes[pItem.rowIndex];
                        pItem.x = pItem.x + (tableView.width - iRowWidth) / 2 + tableView.spacing;
                    }
                }
            }

            Item {
                id: repeaterItem
                width: localTextMetrics.boundingRect.width + 20
                height: tableView.itemHeight
                property int rowIndex: 0
                property string textForButton: ""
                property color bgColor: "#FF000000"
                property color fgColor: "#FFFFFFFF"

                state: "HIDDEN"
                property int delayOnShow: index*300

                states: [
                    State {
                        name: "HIDDEN"
                        PropertyChanges { target: button; height: 0; width: 0 }
                    },
                    State {
                        name: "VISIBLE"
                        PropertyChanges { target: button; height: tableView.itemHeight; width: localTextMetrics.boundingRect.width + 20 }
                    }
                ]

                Component.onCompleted: {
                    // don't connect properties directly otherwise all buttons will
                    // have the same text and on new buttons all texts on all buttons change
                    if (textDelegate.buttonTexts.length > index) {
                        repeaterItem.textForButton = textDelegate.buttonTexts[index];
                    }
                    if (textDelegate.backgroundColors.length > index) {
                        let colObj = textDelegate.backgroundColors[index];
                        repeaterItem.bgColor = Qt.rgba(colObj.r, colObj.g, colObj.b, colObj.a);
                    }
                    if (textDelegate.textColors.length > index) {
                        let colObj = textDelegate.textColors[index];
                        repeaterItem.fgColor = Qt.rgba(colObj.r, colObj.g, colObj.b, colObj.a);
                    }

                    repeaterItem.state = "VISIBLE";
                }

                TextMetrics {
                    id: localTextMetrics
                    font.family: root.currentlyLoadedProject.font
                    font.pointSize: 14
                    text: text.text
                }

                Button {
                    id: button
                    anchors.centerIn: parent
                    text: repeaterItem.textForButton
                                        .replace("<html>","").replace("</html>","")
                                        .replace("<body>", "").replace("</body>", "");
                    z: 1

                    focusPolicy: Qt.NoFocus

                    property string bShortcut: Settings.keyBinding("Answer_" + (index+1))

                    Behavior on width  {
                         animation: SequentialAnimation {
                             PauseAnimation { duration: repeaterItem.delayOnShow }
                             ScriptAction {
                                script: text.opacity = 1
                             }
                             SpringAnimation { spring: 5; damping: 0.5 }
                         }
                     }
                     Behavior on height  {
                         animation: SequentialAnimation {
                             PauseAnimation { duration: repeaterItem.delayOnShow }
                             SpringAnimation { spring: 5; damping: 0.5 }
                         }
                     }

                    onHoveredChanged: {
                        if (hovered)
                        {
                            root.soundEffects.hoverSound.play();
                        }
                    }
                    onClicked: {
                        root.soundEffects.clickSound.play();
                        if (inList) {
                            listView.buttonPressed(index, textDelegate.textContent,
                                                   textDelegate.requestId,
                                                   textDelegate.storeIntoStorageInstead);
                        } else {
                            buttonPressed(index, textDelegate.textContent,
                                          textDelegate.requestId,
                                          textDelegate.storeIntoStorageInstead);
                        }
                    }

                    background: Rectangle {
                        anchors.fill: parent
                        color: repeaterItem.bgColor
                        radius: 5
                    }

                    contentItem: TextItemFormated {
                        id: text
                        anchors.centerIn: parent
                        maximumWidth: textDelegate.width - 50
                        text: repeaterItem.textForButton
                        textColor: repeaterItem.fgColor

                        opacity: 0
                        Behavior on opacity {
                            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
                        }
                    }

                    Text {
                        id: shortcutText
                        visible: text.opacity > 0.8
                        x: -1
                        y: -1
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        font.pointSize: 8
                        font.hintingPreference: Font.PreferNoHinting
                        elide: Text.ElideNone
                        text: button.bShortcut
                        wrapMode: Text.NoWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }


                    Shortcut {
                        sequences: [ button.bShortcut ];
                        context: Qt.ApplicationShortcut
                        onActivated: {
                            root.soundEffects.clickSound.play();
                            if (inList) {
                                listView.buttonPressed(index, textDelegate.textContent,
                                                       textDelegate.requestId,
                                                       textDelegate.storeIntoStorageInstead);
                            } else {
                                buttonPressed(index, textDelegate.textContent,
                                              textDelegate.requestId,
                                              textDelegate.storeIntoStorageInstead);
                            }
                        }
                    }
                }
            }

        }
    }

    Component.onCompleted: {
        if (inList) {
            listView.delegateComponentLoaded();
        } else {
            delegateComponentLoaded();
        }
    }
}
