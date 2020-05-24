import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: root
    color: "transparent"

    property Project currentlyLoadedProject: null
    property PlayerTextBox registeredTextBox: null
    property int numReadyComponents: 0
    property var componentsRegistered: []
    signal startLoadingSkript()

    //------------------------------------------------------------------------------------
    //
    function onResize()
    {
        // nothing to do
    }

    //------------------------------------------------------------------------------------
    //
    // project functions
    function onLoadProject()
    {
        if (null !== currentlyLoadedProject && undefined !== currentlyLoadedProject)
        {
            var bFoundLayout = false;
            if (currentlyLoadedProject.playerLayout !== "")
            {
                var resource = currentlyLoadedProject.resource(currentlyLoadedProject.playerLayout);
                if (null !== resource && undefined !== resource)
                {
                    layoutLoader.source = resource.path;
                    bFoundLayout = true;
                }
            }

            if (!bFoundLayout)
            {
                layoutLoader.source = "qrc:/qml/resources/qml/PlayerDefaultLayout.qml";
            }
        }
    }

    function onUnLoadProject()
    {
        currentlyLoadedProject = null;
    }

    //------------------------------------------------------------------------------------
    //
    // TextBox access
    function clearTextBox()
    {
        if (null !== registeredTextBox && undefined !== registeredTextBox)
        {
            registeredTextBox.clearTextBox();
        }
    }

    signal sceneSelectionReturnValue(int iValue)
    function showSceneSelection(vsInput)
    {
        if (null !== registeredTextBox && undefined !== registeredTextBox)
        {
            registeredTextBox.showSceneSelectionPrompts(vsInput);
        }
    }
    Connections {
        target: registeredTextBox
        onSceneSelectionRetVal: {
            sceneSelectionReturnValue(iValue)
        }
    }

    function showText(sText, vTextColors, vBackgroundColors)
    {
        if (null !== registeredTextBox && undefined !== registeredTextBox)
        {
            var vBCol = registeredTextBox.backgroundColors();
            var vTCol = registeredTextBox.textColors();

            registeredTextBox.setBackgroundColors(vBackgroundColors);
            registeredTextBox.setTextColors(vTextColors);

            registeredTextBox.showText(sText);

            registeredTextBox.setBackgroundColors(vBCol);
            registeredTextBox.setTextColors(vTCol);
        }
    }


    //------------------------------------------------------------------------------------
    //
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

    // this is the project specific dynamic layout
    AnimatedImage {
        id: animation
        anchors.fill: parent
        visible: layoutLoader.status === Loader.Loading || layoutLoader.status === Image.Null
        source: "qrc:/resources/gif/spinner_transparent.gif"
        fillMode: Image.Pad
    }
    Rectangle {
        color: "transparent"
        anchors.fill: parent
        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            horizontalOffset: 8
            verticalOffset: 8
        }

        Loader {
            id: layoutLoader
            anchors.fill: parent
            asynchronous: true
            onStatusChanged: {
                if (status === Loader.Ready)
                {
                    startLoadingSkript();
                }
                else if (status === Loader.Error)
                {
                    console.error(qsTr("Could not load layout."));
                    layoutLoader.source = "qrc:/qml/resources/qml/PlayerDefaultLayout.qml";
                }
            }
        }
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(thread.userName, thread);
        ScriptRunner.registerNewComponent(storage.userName, storage);
        numReadyComponents += 2;
    }
}
