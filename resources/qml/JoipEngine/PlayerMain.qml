import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import QtMultimedia 5.14
import JOIP.core 1.5
import JOIP.db 1.1
import JOIP.script 1.3

import "EvalWrapper.js" as EvalWrapper
import "qrc:/xmldom/dom-parser.mjs" as DOMParser;

Rectangle {
    id: root
    color: "transparent"
    property var style: null

    // make globally accessible for now
    property alias soundEffects: playerSoundEffects

    property bool debug: false
    property Project currentlyLoadedProject: null
    property PlayerTextBox registeredTextBox: null
    property PlayerMediaPlayer registeredMediaPlayer: null
    property int numReadyComponents: 0
    property var componentsRegistered: []
    property var startTime: new Date()

    signal startLoadingSkript()
    signal unloadFinished()
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
            if (!root.debug) {
                var played = saveManager.load("numPlayed", "stats");
                saveManager.store("numPlayed", played == null ? 1 : played+1, "stats");

                root.startTime = new Date();
            }

            audioPlayer.numberOfSoundEmitters = currentlyLoadedProject.numberOfSoundEmitters;

            var bFoundLayout = false;
            if (currentlyLoadedProject.playerLayout !== "")
            {
                var resource = currentlyLoadedProject.resource(currentlyLoadedProject.playerLayout);
                if (null !== resource && undefined !== resource)
                {
                    var path = resource.path;
                    layoutLoader.setSource(path);
                    bFoundLayout = true;
                }
                else if (currentlyLoadedProject.playerLayout.startsWith("qrc:/qml"))
                {
                    layoutLoader.setSource(currentlyLoadedProject.playerLayout);
                    bFoundLayout = true;
                }
            }

            if (!bFoundLayout)
            {
                layoutLoader.setSource("qrc:/qml/resources/qml/JoipEngine/PlayerDefaultLayout.qml");
            }
        }
        else
        {
            console.error(qsTr("Could not load project, project is null or undefined."));
        }
    }

    function onUnLoadProject(bReachedEnd)
    {
        if (null != currentlyLoadedProject)
        {
            if (!root.debug) {
                if (bReachedEnd) {
                    var finished = saveManager.load("numFinished", "stats");
                    saveManager.store("numFinished", finished == null ? 1 : finished+1, "stats");
                }

                var timeDiff = new Date() - root.startTime;
                var playTime = saveManager.load("playTime", "stats");
                saveManager.store("playTime", playTime == null ? timeDiff : playTime+timeDiff, "stats");
            }

            // stop toys
            TeaseDeviceController.sendStopCmd();

            // clear eval environement and storage
            storage.clear();
            SoundManager.clearRegistry();

            registeredTextBox = null;
            registeredMediaPlayer = null;

            // TODO: unload registered eval access objects

            numReadyComponents = 0;
            componentsRegistered = [];

            layoutLoader.setSource("");
            layoutLoader.unload();

            hoverSound.source = "";
            clickSound.source = "";

            audioPlayer.numberOfSoundEmitters = 5;

            currentlyLoadedProject = null;

            style = null;

            gc();

            root.unloadFinished();
        }
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
    onSceneSelectionReturnValue: {
        transitionLoader.unload();
    }
    function showSceneSelection(vsInput, aditionalData)
    {
        if (null == aditionalData || "" === aditionalData)
        {
            if (null !== registeredTextBox && undefined !== registeredTextBox)
            {
                registeredTextBox.showSceneSelectionPrompts(vsInput);
            }
            else
            {
                console.error(qsTr("Could not show scene selection."));
                root.sceneSelectionReturnValue(-1);
            }
        }
        else
        {
            if (null !== currentlyLoadedProject && undefined !== currentlyLoadedProject)
            {
                var resource = currentlyLoadedProject.resource(aditionalData);
                if (null !== resource && undefined !== resource)
                {
                    var path = resource.path;
                    transitionLoader.vsInput = vsInput;
                    transitionLoader.setSource(path);
                }
                else
                {
                    console.error(qsTr("Could not load layout."));
                    root.sceneSelectionReturnValue(-1);
                }
            }
            else
            {
                console.error(qsTr("Could not load layout."));
                root.sceneSelectionReturnValue(-1);
            }
        }
    }
    Connections {
        target: registeredTextBox
        onSceneSelectionRetVal: {
            sceneSelectionReturnValue(iValue);
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
    PlayerAudioPlayer {
        id: audioPlayer

        onPlaybackFinishedCallback: {
            for (var i = 0; root.componentsRegistered.length > i; ++i)
            {
                if (null !== root.componentsRegistered[i] && undefined !== root.componentsRegistered[i])
                {
                    root.componentsRegistered[i].soundFinished(resource);
                }
            }
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
        saveManager: SavegameManager {
            id: saveManager
            project: currentlyLoadedProject

            onAchievementValueChanged: {
                achievements.updateAchievementView(sId, value, oldValue);
            }
        }

        function setItem(sId, value)
        {
            store(sId, value);
        }
        function getItem(sId)
        {
            return load(sId);
        }
        function removeItem(sId)
        {
            store(sId, null);
        }
    }
    StorageSignalEmitter {
        id: storageEmitter
        property string userName: "localStorage"

        onClear: {
            storage.clear();
        }
        onLoad: {
            var loaded = storage.load(sId, sContext);
            loadReturnValue(loaded, sRequestId);
        }
        onLoadPersistent: {
            storage.loadPersistent(sId, "");
        }
        onRemoveData: {
            saveManager.removeData(sId, sContext);
        }
        onStore: {
            storage.store(sId, value, sContext);
        }
        onStorePersistent: {
            storage.storePersistent(sId, "");
        }
    }
    property var deviceController: ({
       pause: function ()
       {
           TeaseDeviceController.pause();
       },
       resume: function()
       {
           TeaseDeviceController.resume();
       },
       sendLinearCmd: function (dDurationS, dPosition)
       {
            TeaseDeviceController.sendLinearCmd(Math.floor(dDurationS*1000), dPosition);
       },
       sendRotateCmd: function(bClockwise, dSpeed)
       {
            TeaseDeviceController.sendRotateCmd(bClockwise, dSpeed);
       },
       sendStopCmd: function()
       {
            TeaseDeviceController.sendStopCmd();
       },
       sendVibrateCmd: function(dSpeed)
       {
            TeaseDeviceController.sendVibrateCmd(dSpeed);
       }
    })
    DeviceControllerSignalEmitter {
        id: deviceControllerEmitter
        property string userName: "deviceController"

        onSendLinearCmd: {
            deviceController.sendLinearCmd(dDurationS, dPosition);
        }
        onSendRotateCmd: {
            deviceController.sendRotateCmd(bClockwise, dSpeed);
        }
        onSendStopCmd: {
            deviceController.sendStopCmd();
        }
        onSendVibrateCmd: {
            deviceController.sendVibrateCmd(dSpeed);
        }
    }
    PlayerSceneManager {
        id: sceneManager
    }

    //------------------------------------------------------------------------------------
    // variables
    // from openeos:
    // Convert HTML string to in-line javascript string expression
    // Replace ...<eval>expression</eval>... with "..." + (isolated eval expression) + "..."
    function parseHtmlToJS(string, context) {
      if (typeof string !== 'string') return JSON.stringify('');
      var result = [];
      var parser = new DOMParser.DOMParser({});
      var doc = parser
        .parseFromString(string, 'text/html')
        .getElementsByTagName('body')[0];
      if (null == doc) return string;
      var evs = doc.getElementsByTagName('eval');
      if (null == evs) return string;
      let docstring = doc.innerHTML;
      docstring = docstring.replace(' xmlns="http://www.w3.org/1999/xhtml"', '');
      if (null == docstring) return string;
      for (var iEv = 0; iEv < evs.length; ++iEv) {
        var ev = evs.item(iEv);
        if (null == ev) continue;
        var evHtml = ev.outerHTML;
        evHtml = evHtml.replace(' xmlns="http://www.w3.org/1999/xhtml"', '');
        var i = docstring.indexOf(evHtml);
        var beforeEv = docstring.slice(0, i);
        var afterEv = docstring.slice(i + evHtml.length, docstring.length);
        if (beforeEv.length) {
          result.push(beforeEv);
        }
        var evExpression = QtApp.decodeHTML(ev.innerHTML).trim();
        if (evExpression.length) {
          result.push(EvalWrapper.globalEval(EvalWrapper.isolate(evExpression, context +' <eval>')));
        }
        docstring = afterEv.replace(' xmlns="http://www.w3.org/1999/xhtml"', '');
      }
      if (docstring.length) {
        result.push(docstring);
      }
      if (!result.length) return JSON.stringify('');
      return '<html><body><nobr>' + result.join('') + '</nobr></body></html>';
    }
    EvalSignalEmitter {
        id: evaluator
        property string userName: "evalRunner"

        onEvalQuery: {
            var retVal = EvalWrapper.globalEval(EvalWrapper.isolate(sScript, 'evaluator <' + userName + '>'));
            evaluator.evalReturn(retVal);
        }
    }
    function evaluate(sScript) {
        return EvalWrapper.globalEval(EvalWrapper.isolate(sScript, 'evaluator <' + evaluator.userName + '>'));
    }

    PlayerBackground {
        id: background
        anchors.fill: parent
    }

    //------------------------------------------------------------------------------------
    //
    // Misc
    function registerUIComponent(sComponent, component)
    {
        EvalWrapper.registerUIComponent(sComponent, component);
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
        anchors.centerIn: parent
        width: parent.width+8 // needed to not have a border because of the dropshadow
        height: parent.height+8 // needed to not have a border because of the dropshadow
        layer.enabled: Settings.playerDropShadow
        layer.effect: DropShadow {
            transparentBorder: true
            horizontalOffset: 8
            verticalOffset: 8
            cached: true
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
            active: false

            visible: opacity > 0.0;
            Behavior on opacity {
                NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
            }

            function unload(){
                sourceComponent = undefined;
                source = "";
                opacity = 0.0;
                active = false;
            }

            onLoaded: {
                if (status === Loader.Ready)
                {
                    root.startLoadingSkript();
                }
            }
            onSourceChanged: {
                if ("" !== source)
                {
                    console.log("Starting to load: " + source);
                    opacity = 1.0;
                    active = true;
                }
            }

            onStatusChanged: {
                switch(status)
                {
                  case Loader.Null: console.log("The loader is inactive or no QML source has been set"); break;
                  case Loader.Ready: console.log("The QML source has been loaded"); break;
                  case Loader.Loading: console.log("The QML source is currently being loaded"); break;
                  case Loader.Error: console.error("An error occurred while loading the QML source"); break;
                }

                if (status === Loader.Error)
                {
                    console.error(qsTr("Could not load layout."));
                }
            }
            onProgressChanged: {
                console.log("Loading Layout: " + progress + "%");
            }
        }

        // this is the custom scene transition
        Loader {
            id: transitionLoader
            anchors.fill: parent
            asynchronous: true
            active: false

            visible: opacity > 0.0;
            Behavior on opacity {
                NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
            }

            property var vsInput: []

            function unload(){
                sourceComponent = undefined;
                source = "";
                opacity = 0.0;
                active = false;
                vsInput = [];
            }

            onSourceChanged: {
                if ("" !== source)
                {
                    console.log("Starting to load: " + source);
                    opacity = 1.0;
                    active = true;
                }
            }

            onStatusChanged: {
                switch(status)
                {
                  case Loader.Null: console.log("The loader is inactive or no QML source has been set"); break;
                  case Loader.Ready: console.log("The QML source has been loaded"); break;
                  case Loader.Loading: console.log("The QML source is currently being loaded"); break;
                  case Loader.Error: console.error("An error occurred while loading the QML source"); break;
                }

                if (status === Loader.Error)
                {
                    console.error(qsTr("Could not load layout."));
                    root.sceneSelectionReturnValue(-1);
                }
            }
            onProgressChanged: {
                console.log("Loading Layout: " + progress + "%");
            }
        }
    }

    PlayerAchievementView {
        id: achievements
        project: currentlyLoadedProject

        width: Math.min(itemHeight*3, root.width)
        height: itemHeight*3

        x: Settings.dominantHand === DominantHand.Right ? 5 : parent.width - width - 5
        y: parent.height - height - 5
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
        ScriptRunner.registerNewComponent(deviceControllerEmitter.userName, deviceControllerEmitter);
        ScriptRunner.registerNewComponent(evaluator.userName, evaluator);
        numReadyComponents += 4;

        registerUIComponent("teaseStorage", storage);
        registerUIComponent("localStorage", storage);
        registerUIComponent("deviceController", deviceController);
        evaluate("var window = {};");
    }

    // handle interrupt
    Connections {
        target: ScriptRunner
        onRunningChanged: {
            if (!bRunning)
            {
                deviceController.pause();
            }
            else
            {
                deviceController.resume();
            }
        }
    }
}
