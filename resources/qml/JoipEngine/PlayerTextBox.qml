import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.5
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: textBox
    color: "transparent"

    // Define mode enum
    enum TextBoxMode {
        Log = 0,
        TextBox = 1
    }

    property string userName: "textBox"
    property bool mainTextBox: false
    property int iconWidth: 64
    property int iconHeight: 64
    property int displayMode: PlayerTextBox.TextBoxMode.Log
    property bool logShown: false
    property bool hideLogAfterInactivity: false
    property int hideTimerIntervalMS: Settings.hideSettingsTimeout

    function clearTextBox()
    {
        boxLoader.setSource("");
        boxLoaderInput.setSource("");
        textLogModel.clear();
        textLogModel.append({
            "defaultText": "",
            "textAlignment": "",
            "textContent": "",
            "buttonTexts": [],
            "storeIntoStorageInstead": false,
            "backgroundColor": "",
            "textColor": "",
            "backgroundColors": [],
            "textColors": [],
            "numButtons": "",
            "portrait": null,
            "sRequestId": "",
            "type": "TextBoxSpacerDelegate.qml"
        });
        textLog.cancelFlick();
        textLog.flick(0,-1000);
    }

    function showButtonPrompts(vsLabels, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
    {
        dataContainer.buttonTexts = vsLabels;
        var colObjBg = dataContainer.backgroundColors;
        var colObjTx = dataContainer.textColors;
        var dataForInput = {
            "defaultText": "",
            "textAlignment": "",
            "textContent": sStoreIntoVar,
            "buttonTexts": vsLabels,
            "storeIntoStorageInstead": bStoreIntoStorageInstead,
            "backgroundColor": "",
            "textColor": "",
            "backgroundColors": JSON.stringify(dataContainer.backgroundColors), /*needed becase QML ListModel can't work with arrays of objects*/
            "textColors": JSON.stringify(dataContainer.textColors), /*needed becase QML ListModel can't work with arrays of objects*/
            "portrait": null,
            "sRequestId": sRequestId,
            "type": "TextBoxButtonsDelegate.qml"
        };
        var dataForLoader = {
            "defaultText": "",
            "buttonTexts": vsLabels,
            "textContent": sStoreIntoVar,
            "storeIntoStorageInstead": bStoreIntoStorageInstead,
            "backgroundColors": dataContainer.backgroundColors,
            "textColors": dataContainer.textColors,
            "requestId": sRequestId
        };
        if (PlayerTextBox.TextBoxMode.Log === textBox.displayMode)
        {
            textLogModel.append(dataForInput);
            textLog.cancelFlick();
            textLog.flick(0,-1000);
        }
        else
        {
            boxLoaderInput.setSource("TextBoxButtonsDelegate.qml", dataForLoader);
        }
    }

    signal sceneSelectionRetVal(int iValue)
    function showSceneSelectionPrompts(vsLabels)
    {
        dataContainer.sceneSelection = true;
        showButtonPrompts(vsLabels);
    }

    function showInput(sDefault, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
    {
        var dataForInput = {
            "defaultText": sDefault,
            "textAlignment": "",
            "textContent": sStoreIntoVar,
            "buttonTexts": [],
            "storeIntoStorageInstead": bStoreIntoStorageInstead,
            "backgroundColor": undefined !== dataContainer.backgroundColors  && dataContainer.backgroundColors.length > 0 ? dataContainer.backgroundColors[0] : "#ff000000",
            "textColor": undefined !== dataContainer.textColors  && dataContainer.textColors.length > 0 ? dataContainer.textColors[0] : "#ffffffff",
            "backgroundColors": [],
            "textColors": [],
            "numButtons": "",
            "portrait": null,
            "sRequestId": sRequestId,
            "type": "TextBoxInputDelegate.qml"
        };
        var dataForLoader = {
            "defaultText": sDefault,
            "textContent": sStoreIntoVar,
            "storeIntoStorageInstead": bStoreIntoStorageInstead,
            "backgroundColor": undefined !== dataContainer.backgroundColors  && dataContainer.backgroundColors.length > 0 ? dataContainer.backgroundColors[0] : "#ff000000",
            "textColor": undefined !== dataContainer.textColors  && dataContainer.textColors.length > 0 ? dataContainer.textColors[0] : "#ffffffff",
            "requestId": sRequestId
        };
        if (PlayerTextBox.TextBoxMode.Log === textBox.displayMode)
        {
            textLogModel.append(dataForInput);
            textLog.cancelFlick();
            textLog.flick(0,-1000);
        }
        else
        {
            boxLoaderInput.setSource("TextBoxInputDelegate.qml", dataForLoader);
        }
    }

    function showText(sText, onlyInList)
    {
        var dataForList = {
            "defaultText": "",
            "textAlignment": dataContainer.textAlignment,
            "textContent": sText,
            "buttonTexts": [],
            "storeIntoStorageInstead": false,
            "backgroundColor": undefined !== dataContainer.backgroundColors  && dataContainer.backgroundColors.length > 0 ? dataContainer.backgroundColors[0] : "#ff000000",
            "textColor": undefined !== dataContainer.textColors  && dataContainer.textColors.length > 0 ? dataContainer.textColors[0] : "#ffffffff",
            "backgroundColors": [],
            "textColors": [],
            "numButtons": "",
            "portrait": dataContainer.portrait,
            "sRequestId": "",
            "type": "TextBoxTextDelegate.qml"
        };
        var dataForLoader = {
            "defaultText": "",
            "textAlignment": dataContainer.textAlignment,
            "textContent": sText,
            "backgroundColor": undefined !== dataContainer.backgroundColors  && dataContainer.backgroundColors.length > 0 ? dataContainer.backgroundColors[0] : "#ff000000",
            "textColor": undefined !== dataContainer.textColors  && dataContainer.textColors.length > 0 ? dataContainer.textColors[0] : "#ffffffff",
            "portrait": dataContainer.portrait
        };
        textLogModel.append(dataForList);
        textLog.cancelFlick();
        textLog.flick(0,-1000);
        if (!onlyInList)
        {
            boxLoader.setSource("TextBoxTextDelegate.qml", dataForLoader);
            boxLoaderInput.setSource("");
        }
    }

    function setPortrait(sName)
    {
        if (null !== registrator.currentlyLoadedProject &&
            undefined !== registrator.currentlyLoadedProject)
        {
            if (sName === "")
            {
                dataContainer.portrait = null;
            }
            else
            {
                var pResource = registrator.currentlyLoadedProject.resource(sName);
                if (null !== pResource && undefined !== pResource)
                {
                    dataContainer.portrait = pResource;
                }
                else
                {
                    dataContainer.portrait = null;
                }
            }
        }
    }

    function backgroundColors()
    {
        return dataContainer.backgroundColors;
    }
    function setBackgroundColors(vColors)
    {
        dataContainer.backgroundColors = vColors;
    }

    function textColors()
    {
        return dataContainer.textColors;
    }
    function setTextColors(vColors)
    {
        dataContainer.textColors = vColors;
    }

    // accessor object for eval
    property var evalAccessor: ({
        clearTextBox: function()
        {
            textBox.clearTextBox();
        },
        showButtonPrompts: function(vsLabels, sStoreIntoVarOrId, sRequestId, bStoreIntoStorageInstead)
        {
            if (null == sRequestId) {
                // compatibility for teases before 1.4.1
                signalEmitter.showButtonPrompts(vsLabels, null, sStoreIntoVarOrId, false);
            } else {
                signalEmitter.showButtonPrompts(vsLabels, sStoreIntoVarOrId, sRequestId, bStoreIntoStorageInstead);
            }
        },
        showSceneSelectionPrompts: function(vsLabels)
        {
            textBox.showButtonPrompts(vsLabels);
        },
        //sDefault nees to be in last place because of EOS compatibility
        showInput: function(sStoreIntoVar, sRequestId, bStoreIntoStorageInstead, sDefault = "")
        {
            signalEmitter.showInput(sDefault, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead);
        },
        showText: function(sText)
        {
            signalEmitter.showText(sText, -1);
        },
        setPortrait: function(sName)
        {
            textBox.setPortrait(sName);
        },
        backgroundColors: function()
        {
            return textBox.backgroundColors();
        },
        setBackgroundColors: function(vColors)
        {
            textBox.setBackgroundColors(vColors);
        },
        textColors: function()
        {
            return textBox.textColors();
        },
        setTextColors: function(vColors)
        {
            textBox.setTextColors(vColors);
        }
    });

    // signal handling from script
    TextBoxSignalEmitter {
        id: signalEmitter

        onClearText: {
            textBox.clearTextBox();
        }
        onGetDialogue: {
            var dialogue = null;
            if ("" !== sId)
            {
                if (bIsRegexp)
                {
                    var dialgsRx = DialogueManager.dialogueFromRx(sId);
                    dialogue = dialgsRx[Math.floor(Math.random() * dialgsRx.length)];
                }
                else
                {
                    dialogue = DialogueManager.dialogue(sId);
                }
            }
            else if (vsTags.length > 0)
            {
                var dialgs = DialogueManager.dialogueFromTags(vsTags);
                dialogue = dialgs[Math.floor(Math.random() * dialgs.length)];
            }

            var dialogueData = null;
            if (null != dialogue)
            {
                if (dialogue.hasCondition && dialogue.numDialogueData() > 0)
                {
                    dialogueData = dialogue.dialogueData(0);
                }
                else if (!dialogue.hasCondition && dialogue.numDialogueData() > 0)
                {
                    for (var i = 0; dialogue.numDialogueData() > i; ++i)
                    {
                        var data = dialogue.dialogueData(i);
                        if (root.evaluate(data.condition))
                        {
                            dialogueData = data;
                            break;
                        }
                    }
                }
            }

            if (null != dialogueData)
            {
                signalEmitter.getDialogueReturnValue(sRequestId, dialogueData.string,
                                                     dialogueData.waitTimeMs, dialogueData.skipable,
                                                     dialogueData.soundResource,
                                                     dialogue.tags());
            }
            else
            {
                signalEmitter.getDialogueReturnValue(sRequestId, "", -1, false, "", []);
            }
        }
        onShowButtonPrompts: {
            var vsModifiedPrompts = vsLabels;
            for (var i = 0; i < vsModifiedPrompts.length; ++i) {
                if (vsModifiedPrompts[i].startsWith("<html>") && vsModifiedPrompts[i].endsWith("</html>")) {
                    vsModifiedPrompts[i] = root.parseHtmlToJS(vsModifiedPrompts[i],  textBox.userName);
                }
            }
            textBox.showButtonPrompts(vsModifiedPrompts, sStoreIntoVar, sRequestId,
                                      bStoreIntoStorageInstead);
            hideTimer.reset(textBox.hideTimerIntervalMS);
        }
        onShowInput: {
            textBox.showInput(sDefault, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead);
            hideTimer.reset(textBox.hideTimerIntervalMS);
        }
        onShowText: {
            if (sText.startsWith("<html>") && sText.endsWith("</html>")) {
                textBox.showText(root.parseHtmlToJS(sText, textBox.userName), false);
            } else {
                textBox.showText(sText, false);
            }
            if (0 < dSkippableWaitS) {
                registrator.setSkippableWait(dSkippableWaitS);
            }
            if ("" !== sResource) {
                var pResource = registrator.currentlyLoadedProject.resource(sResource);
                if (null !== pResource && undefined !== pResource &&
                    Resource.Sound === pResource.type)
                {
                    registrator.playAudio(sResource, pResource, 1, 0, -1);
                }
            }

            hideTimer.reset((textBox.hideTimerIntervalMS < dSkippableWaitS*1000 ? dSkippableWaitS*1000 : textBox.hideTimerIntervalMS));
        }
        onStopResource: {
            registrator.tryToCallAudio(sResource, "stop");
        }
        onTextAlignmentChanged: {
            dataContainer.textAlignment = alignment;
        }
        onTextBackgroundColorsChanged: {
            setBackgroundColors(vColors);
        }
        onTextColorsChanged: {
            setTextColors(vColors);
        }
        onTextPortraitChanged: {
            setPortrait(sResource);
        }
    }

    PlayerComponentRegistrator {
        id: registrator

        onSkippableWait: {
            dataContainer.skippable = true;
        }
        onSkippableWaitFinished: {
            dataContainer.skippable = false;
            signalEmitter.waitSkipped();
        }
        onSoundFinished: {
            signalEmitter.soundFinished(sResource);
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: textBox.hideLogAfterInactivity
        onPositionChanged: {
            hideTimer.reset(textBox.hideTimerIntervalMS);
        }
    }

    Item {
        id: dataContainer
        property var backgroundColors: [ Qt.rgba(0, 0, 0, 1) ]
        property var textColors: [ Qt.rgba(1, 1, 1, 1) ]
        property var buttonTexts: []
        property var textAlignment: TextAlignment.AlignCenter
        property Resource portrait: null
        property bool sceneSelection: false
        property bool skippable: false

        function inputEditingFinished(sInput, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
        {
            if (null != sStoreIntoVar && "" !== sStoreIntoVar)
            {
                if (bStoreIntoStorageInstead)
                {
                    root.evaluate("teaseStorage.store('"+sStoreIntoVar+"',"+sInput+");");
                }
                else
                {
                    root.evaluate(sStoreIntoVar + "='" + sInput + "'");
                }
            }
            signalEmitter.showInputReturnValue(sInput, sRequestId);
        }
        function buttonPressed(iIndex, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
        {
            if (null != sStoreIntoVar && "" !== sStoreIntoVar)
            {
                if (bStoreIntoStorageInstead)
                {
                    root.evaluate("teaseStorage.store('"+sStoreIntoVar+"',"+iIndex+");");
                }
                else
                {
                    root.evaluate(sStoreIntoVar + "='" + iIndex + "'");
                }
            }

            if (!sceneSelection)
            {
                signalEmitter.showButtonReturnValue(iIndex, sRequestId);
            }
            else
            {
                sceneSelection = false;
                textBox.sceneSelectionRetVal(iIndex);
            }
        }
    }

    // GUI:
    // first is the textBox
    Rectangle {
        color: "transparent"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: parent.width
        height: parent.height * 2 / 3

        Loader {
            id: boxLoader
            anchors.fill: parent
            asynchronous: true

            active: PlayerTextBox.TextBoxMode.TextBox === textBox.displayMode
            visible: PlayerTextBox.TextBoxMode.TextBox === textBox.displayMode

            property var buttonTexts: dataContainer.buttonTexts
            property bool sceneSelection: dataContainer.sceneSelection
            property bool skippable: dataContainer.skippable
            property int iconWidth: textBox.iconWidth
            property int iconHeight: textBox.iconHeight
            property var model: null

            signal delegateComponentLoaded()
            onDelegateComponentLoaded: {
                // nothing to do anymore
            }
        }

        // animations
        Timer {
            id: hideTimer
            interval: textBox.hideTimerIntervalMS
            running: false
            repeat: false
            function reset(interv) {
                if (textBox.hideLogAfterInactivity) {
                    hideTimer.interval = interv;
                    parent.opacity = 1
                    restart();
                }
            }
            onTriggered: {
                parent.opacity = 0.3
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
        }
    }
    // second is the input Box
    Loader {
        id: boxLoaderInput
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height / 2
        asynchronous: true

        active: PlayerTextBox.TextBoxMode.TextBox === textBox.displayMode
        visible: PlayerTextBox.TextBoxMode.TextBox === textBox.displayMode

        property var buttonTexts: dataContainer.buttonTexts
        property bool sceneSelection: dataContainer.sceneSelection
        property bool skippable: dataContainer.skippable
        property int iconWidth: textBox.iconWidth
        property int iconHeight: textBox.iconHeight
        property var model: null

        signal delegateComponentLoaded()
        onDelegateComponentLoaded: {
            // nothing to do anymore
        }

        function inputEditingFinished(sInput, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
        {
            dataContainer.inputEditingFinished(sInput, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead);
            if (PlayerTextBox.TextBoxMode.TextBox === textBox.displayMode)
            {
                textBox.showText(sInput, true);
            }
        }
        function buttonPressed(iIndex, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
        {
            dataContainer.buttonPressed(iIndex, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead);
            if (PlayerTextBox.TextBoxMode.TextBox === textBox.displayMode)
            {
                textBox.showText(dataContainer.buttonTexts[iIndex], true);
            }
        }
    }
    // second is the log
    ListView {
        id: textLog
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: PlayerTextBox.TextBoxMode.Log === textBox.displayMode ? parent.bottom :
                                                                                parent.top
        width: parent.width
        height: visible ? parent.height : 0

        Behavior on height {
            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
        }

        visible: PlayerTextBox.TextBoxMode.Log === textBox.displayMode || textBox.logShown

        cacheBuffer: 0
        orientation: ListView.Vertical
        clip: true

        property var buttonTexts: dataContainer.buttonTexts
        property bool sceneSelection: dataContainer.sceneSelection
        property bool skippable: dataContainer.skippable
        property int iconWidth: textBox.iconWidth
        property int iconHeight: textBox.iconHeight

        function inputEditingFinished(sInput, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
        {
            dataContainer.inputEditingFinished(sInput, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead);
        }
        function buttonPressed(iIndex, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
        {
            dataContainer.buttonPressed(iIndex, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
        }

        signal delegateComponentLoaded()
        onDelegateComponentLoaded: {
            // nothing to do anymore
        }

        model: ListModel {
            id: textLogModel
            dynamicRoles: true
        }

        delegate: Component {
            Loader {
                source: type
                asynchronous: false
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: textScrollBar
            visible: true
        }
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        textBox.clearTextBox();
        if (mainTextBox)
        {
            registrator.registerTextBox(textBox);
        }
        registrator.componentLoaded();

        root.registerUIComponent(textBox.userName, evalAccessor);
    }
}
