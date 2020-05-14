import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1

Rectangle {
    id: textDelegate
    width: parent.ListView.view.width
    height: textMetrics.boundingRect.height * tableView.rowCount + 40
    color: "transparent"

    property var backgroundColors: []
    property var textColors: []
    property var buttonTexts: []

    TextMetrics {
        id: textMetrics
        font.family: Settings.font
        font.pointSize: 14
        text: "testText here"
    }

    Flow {
        id: tableView
        spacing: 20

        property int itemHeight: textMetrics.boundingRect.height + 10
        property int itemWithEstimate: textMetrics.boundingRect.width

        property int rowMaxCount: parent.width / (itemWithEstimate + spacing)
        property int rowCount: rowMaxCount > numButtons ? numButtons : rowMaxCount
        property int rowWidth: rowCount * itemWithEstimate + (rowCount - 1) * spacing
        property int rowMar: (parent.width - rowWidth) / 2
        property int colMaxCount: parent.height / (itemHeight + spacing)
        property int colCount: numButtons / rowMaxCount + 1
        property int colHeight: colCount * itemHeight + (colCount - 1) * spacing
        property int colMar: (parent.height - colHeight) / 2

        anchors {
            fill: parent
            leftMargin: rowMar
            rightMargin: rowMar
            topMargin: colMar
            bottomMargin: colMar
        }

        Repeater {
            id: repeater
            model: 0
            Item {
                id: repeaterItem
                width: localTextMetrics.boundingRect.width + 20
                height: tableView.itemHeight

                TextMetrics {
                    id: localTextMetrics
                    font.family: text.font.family
                    font.pointSize: text.font.pointSize
                    text: text.text
                }

                Button {
                    id: button
                    anchors.fill: parent
                    text: textDelegate.buttonTexts[index];

                    onClicked: {
                        textDelegate.parent.ListView.view.buttonPressed(index);
                    }

                    background: Rectangle {
                        anchors.fill: parent
                        color: textDelegate.backgroundColors.length > index ? textDelegate.backgroundColors[index] : "#FF000000"
                        radius: 5
                    }

                    contentItem: Text {
                        id: text
                        anchors.centerIn: parent
                        font.family: Settings.font;
                        font.pointSize: 14
                        elide: Text.ElideNone
                        text: textDelegate.buttonTexts[index]
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: textDelegate.textColors.length > index ? textDelegate.textColors[index] : "#FFFFFFFF"
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
