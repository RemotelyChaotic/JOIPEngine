import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.3
import JOIP.db 1.1

Rectangle {
    id: trans
    anchors.fill: parent
    color: "transparent"

    property Project currentlyLoadedProject: null
    property var vsInput: parent.vsInput

    function returnValue(iValue)
    {
        root.sceneSelectionReturnValue(iValue);
    }

    Component.onCompleted: {
        currentlyLoadedProject = root.currentlyLoadedProject;
    }
}
