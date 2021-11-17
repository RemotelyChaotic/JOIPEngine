import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import QtMultimedia 5.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: root
    color: "transparent"
    property var style: null

    // make globally accessible for now
    property alias soundEffects: playerSoundEffects

    property Project currentlyLoadedProject: null
    property PlayerTextBox registeredTextBox: null
    property PlayerMediaPlayer registeredMediaPlayer: null
    property int numReadyComponents: 0
    property var componentsRegistered: []
    signal startLoadingSkript()
    signal quit()

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
                    layoutLoader.setSource(resource.path);
                    bFoundLayout = true;
                }
            }

            if (!bFoundLayout)
            {
                layoutLoader.setSource("qrc:/qml/resources/qml/PlayerDefaultLayout.qml");
            }
        }
        else
        {
            console.error(qsTr("Could not load project, project is null or undefined."));
        }
    }

    function onUnLoadProject()
    {
        storage.clear();

        registeredTextBox = null;
        registeredMediaPlayer = null;

        numReadyComponents = 0;
        componentsRegistered = [];

        layoutLoader.setSource("");

        currentlyLoadedProject = null;

        style = null;

        gc();
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
    // MediaPlayer access
    function showMedia(sName)
    {
        if (null !== registeredMediaPlayer && undefined !== registeredMediaPlayer)
        {
            registeredMediaPlayer.showOrPlayMedia(sName);
        }
    }
    Connections {
        target: root
        onStartLoadingSkript: {
            if (null !== currentlyLoadedProject && undefined !== currentlyLoadedProject)
            {
                showMedia(currentlyLoadedProject.titleCard);
            }
        }
    }

    //------------------------------------------------------------------------------------
    //
    // Sound
    Item {
        id: playerSoundEffects
        property alias hoverSound: hoverSound
        property alias clickSound: clickSound

        SoundEffect {
            id: hoverSound
            source: "qrc:/resources/sound/menu_selection_soft.wav"
            volume: Settings.volume
            muted: Settings.muted
        }
        SoundEffect {
            id: clickSound
            source: "qrc:/resources/sound/menu_click_soft_main.wav"
            volume: Settings.volume
            muted: Settings.muted
        }
    }

    //------------------------------------------------------------------------------------
    //
    // Basic Components that every project has inherently
    ThreadSignalEmitter {
        id: thread
        property string userName: "thread"

        onSkippableWait: {
            for (var i = 0; root.componentsRegistered.length > i; ++i)
            {
                if (null !== root.componentsRegistered[i] && undefined !== root.componentsRegistered[i])
                {
                    root.componentsRegistered[i].setSkippableWait(iTimeS);
                }
            }
        }
    }
    TeaseStorage {
        id: storage
    }
    StorageSignalEmitter {
        id: storageEmitter
        property string userName: "localStorage"

        onClear: {
            storage.clear();
        }
        onLoad: {
            var loaded = storage.load(sId);
            loadReturnValue(loaded);
        }
        onStore: {
            storage.store(sId, value);
        }
    }
    PlayerBackground {
        id: background
        anchors.fill: parent
    }

    function skipWait()
    {
        for (var i = 0; root.componentsRegistered.length > i; ++i)
        {
            if (null !== root.componentsRegistered[i] && undefined !== root.componentsRegistered[i])
            {
                root.componentsRegistered[i].skippableWaitFinished();
            }
        }
        thread.waitSkipped();
    }

    Shortcut {
        sequence: Settings.keyBinding("Skip")
        onActivated: {
            root.skipWait();
        }
    }

    //------------------------------------------------------------------------------------
    //
    // UI
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

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.skipWait();
            }
        }

        // this is the project specific dynamic layout
        Loader {
            id: layoutLoader
            anchors.fill: parent
            asynchronous: true
            active: true
            onLoaded: {
                if (status === Loader.Ready)
                {
                    root.startLoadingSkript();
                }
            }
            onStatusChanged: {
                switch(status)
                {
                  case Loader.Null: console.log("The loader is inactive or no QML source has been set"); break;
                  case Loader.Ready: console.log("The QML source has been loaded"); break;
                  case Loader.Loading: console.log("The QML source is currently being loaded"); break;
                  case Loader.Error: console.log("An error occurred while loading the QML source"); break;
                }

                if (status === Loader.Error)
                {
                    console.error(qsTr("Could not load layout."));
                    layoutLoader.setSource("qrc:/qml/resources/qml/PlayerDefaultLayout.qml");
                }
            }
            onProgressChanged: {
                console.log("Loading Layout: " + progress + "%");
            }
        }

        PlayerControls {
            id: sceneControl
            color: "transparent"
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 50
            width: parent.width - 100
            height: 64

            soundEffects: playerSoundEffects
        }
    }

    //------------------------------------------------------------------------------------
    //
    // Loadtime initialization
    Component.onCompleted: {
        var styleComponent = Qt.createComponent(Settings.styleFolderQml() + "/Style.qml");
        if (styleComponent.status === Component.Ready)
        {
            root.style = styleComponent.createObject(root);
            if (root.style === null) {
                console.error(qsTr("Error creating Style object"));
            }
        }

        ScriptRunner.registerNewComponent(thread.userName, thread);
        ScriptRunner.registerNewComponent(storageEmitter.userName, storageEmitter);
        numReadyComponents += 2;
    }
}
