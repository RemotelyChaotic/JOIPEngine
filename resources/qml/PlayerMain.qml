import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: root
    color: "transparent"

    property Project currentlyLoadedProject: null
    property int numReadyComponents: 0
    signal startLoadingSkript()

    function onResize()
    {
        // nothing to do
    }

    function onLoadProject()
    {
        if (null !== currentlyLoadedProject && undefined !== currentlyLoadedProject)
        {
            startLoadingSkript();
        }
    }

    function onUnLoadProject()
    {
        currentlyLoadedProject = null;
    }

    // Basic Components that every project has inherently
    ThreadSignalEmitter {
        id: thread
        property string userName: "thread"
    }
    StorageSignalEmitter {
        id: storage
        property string userName: "storage"
    }
    PlayerBackground {
        id: background
        anchors.fill: parent
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(thread.userName, thread);
        ScriptRunner.registerNewComponent(storage.userName, storage);
        numReadyComponents += 2;
    }
}
