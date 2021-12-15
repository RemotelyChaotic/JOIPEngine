import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import QtMultimedia 5.14
import JOIP.core 1.2
import JOIP.db 1.1
import JOIP.script 1.2

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
        // clear eval environement and storage
        storage.clear();

        registeredTextBox = null;
        registeredMediaPlayer = null;

        // TODO: unload registered eval access objects

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

        function setItem(sId, value)
        {
            store(sId, value);
        }
        function getItem(sId)
        {
            return load(sId);
        }
    }
    StorageSignalEmitter {
        id: storageEmitter
        property string userName: "localStorage"

        onClear: {
            storage.clear();
        }
        onLoad: {
            var loaded = storage.load(sId);
            loadReturnValue(loaded, sRequestId);
        }
        onStore: {
            storage.store(sId, value);
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
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: (null != style) ? style.controllButtonDisplay.topOffset : 50
            width: (null != style) ? (parent.width - style.controllButtonDisplay.rightOffset*2) : parent.width-100
            height: (null != style) ? style.controllButtonDisplay.height : 64

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
        ScriptRunner.registerNewComponent(evaluator.userName, evaluator);
        numReadyComponents += 3;

        registerUIComponent("teaseStorage", storage);
    }
}
