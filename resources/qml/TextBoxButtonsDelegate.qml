import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1

Rectangle {
    id: textDelegate
    width: parent.ListView.view.width
    height: tableView.itemHeight * (tableView.rowCount+1) + tableView.spacing * tableView.rowCount + 40
    color: "transparent"

    property var backgroundColors: []
    property var textColors: []
    property var buttonTexts: []

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
            model: 0

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
                property string bgColor: "#FF000000"
                property string fgColor: "#FFFFFFFF"

                Component.onCompleted: {
                    // don't connect properties directly otherwise all buttons will
                    // have the same text and on new buttons all texts on all buttons change
                    if (textDelegate.buttonTexts.length > index) {
                        repeaterItem.textForButton = textDelegate.buttonTexts[index];
                    }
                    if (textDelegate.backgroundColors.length > index) {
                        repeaterItem.bgColor = textDelegate.backgroundColors[index];
                    }
                    if (textDelegate.textColors.length > index) {
                        repeaterItem.fgColor = textDelegate.textColors[index];
                    }
                }

                TextMetrics {
                    id: localTextMetrics
                    font.family: text.font.family
                    font.pointSize: text.font.pointSize
                    text: text.text
                }

                Button {
                    id: button
                    anchors.fill: parent
                    text: repeaterItem.textForButton;
                    z: 1

                    onHoveredChanged: {
                        if (hovered)
                        {
                            root.soundEffects.hoverSound.play();
                        }
                    }
                    onClicked: {
                        root.soundEffects.clickSound.play();
                        textDelegate.parent.ListView.view.buttonPressed(index, model.sRequestId);
                    }

                    background: Rectangle {
                        anchors.fill: parent
                        color: repeaterItem.bgColor
                        radius: 5
                    }

                    contentItem: Text {
                        id: text
                        anchors.centerIn: parent
                        font.family: root.currentlyLoadedProject.font;
                        font.pointSize: textMetrics.font.pointSize
                        font.hintingPreference: Font.PreferNoHinting
                        elide: Text.ElideNone
                        text: repeaterItem.textForButton
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: repeaterItem.fgColor
                        textFormat: QtApp.mightBeRichtext(text) ? Text.RichText : Text.PlainText
                    }

                    Shortcut {
                        sequence: Settings.keyBinding("Answer_" + (index+1));
                        onActivated: {
                            root.soundEffects.clickSound.play();
                            textDelegate.parent.ListView.view.buttonPressed(index);
                        }
                    }
                }
            }

        }
    }

    Component.onCompleted: {
        backgroundColors = textDelegate.parent.ListView.view.backgroundColors;
        textColors = textDelegate.parent.ListView.view.textColors;
        buttonTexts = textDelegate.parent.ListView.view.buttonTexts;
        repeater.model = buttonTexts;
        textDelegate.parent.ListView.view.delegateComponentLoaded();
    }
}
