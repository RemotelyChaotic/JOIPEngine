import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2

Rectangle {
    id: flow
    color: "transparent"

    property int spacing: 20
    property int itemHeight: 20
    property int rowCount: repeater.rowCount

    property alias delegate: repeater.delegate
    property alias model: repeater.model

    function recalculateItems() {
        repeater.recalculateItems();
    }

    onWidthChanged: {
        repeater.recalculateItems();
    }
    onHeightChanged: {
        repeater.recalculateItems();
    }

    Repeater {
        id: repeater
        model: 0

        property int currentRowWidth: 0
        property int numAddedItems: 0
        property int rowCount: 0
        property var rowSizes: []
        property var itemCounter: 0
        property var rowIndex: []

        // we need to calculate positions manually
        onItemAdded: {
            addItemAndDoCalc(item);
        }

        function recalculateItems() {
            currentRowWidth = 0;
            numAddedItems = 0;
            rowCount = 0;
            rowSizes = [];
            itemCounter = 0;
            rowIndex = [];
            for (var i = 0; count > i; ++i) {
                addItemAndDoCalc.call(this, itemAt(i));
            }
        }

        function addItemAndDoCalc(item) {
            numAddedItems++;
            item.x = currentRowWidth;
            if (currentRowWidth + item.width + flow.spacing > flow.width)
            {
                rowSizes[rowCount] = currentRowWidth - flow.spacing;
                rowCount++;
                item.x = 0;
                currentRowWidth = item.width + flow.spacing;
            }
            else
            {
                currentRowWidth += item.width + flow.spacing;
            }
            item.y = rowCount * flow.itemHeight + rowCount * flow.spacing;
            rowIndex[itemCounter] = rowCount;
            rowSizes[rowCount] = currentRowWidth - flow.spacing;

            // all items added
            if (numAddedItems === count)
            {
                for (var i = 0; i < count; ++i)
                {
                    var pItem = itemAt(i);
                    var iRowWidth = rowSizes[rowIndex[i]];
                    var iContentHeight = (rowCount+1)*flow.itemHeight + (rowCount)*flow.spacing;
                    pItem.x = pItem.x + (flow.width - iRowWidth) / 2;
                    pItem.y = pItem.y + (flow.height - iContentHeight) / 2;
                }
            }
            ++itemCounter;
        }
    }
}
