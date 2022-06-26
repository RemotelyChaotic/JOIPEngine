import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: textBox
    color: "transparent"
    property string userName: "textBox"
    property bool mainTextBox: false
    property int iconWidth: 64
    property int iconHeight: 64

    function clearTextBox()
    {
        textLogModel.clear();
        textLogModel.append({
            "textAlignment": "",
            "textContent": "",
            "storeIntoStorageInstead": false,
            "backgroundColor": "",
            "textColor": "",
            "numButtons": "",
            "portrait": null,
            "sRequestId": "",
            "type": "TextBoxSpacerDelegate.qml"
        });
        textLog.cancelFlick();
        textLog.flick(0,-1000);
    }

    function showButtonPrompts(vsLabels, sRequestId)
    {
        textLog.buttonTexts = vsLabels;
        textLogModel.append({
            "textAlignment": "",
            "textContent": "",
            "storeIntoStorageInstead": false,
            "backgroundColor": "",
            "textColor": "",
            "numButtons": vsLabels.length,
            "portrait": null,
            "sRequestId": sRequestId,
            "type": "TextBoxButtonsDelegate.qml"
        });
        textLog.cancelFlick();
        textLog.flick(0,-1000);
    }

    signal sceneSelectionRetVal(int iValue)
    function showSceneSelectionPrompts(vsLabels)
    {
        textLog.sceneSelection = true;
        showButtonPrompts(vsLabels);
    }

    function showInput(sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
    {
        textLogModel.append({
            "textAlignment": "",
            "textContent": sStoreIntoVar,
            "storeIntoStorageInstead": bStoreIntoStorageInstead,
            "backgroundColor": undefined !== textLog.backgroundColors  && textLog.backgroundColors.length > 0 ? textLog.backgroundColors[0] : "#ff000000",
            "textColor": undefined !== textLog.textColors  && textLog.textColors.length > 0 ? textLog.textColors[0] : "#ffffffff",
            "numButtons": "",
            "portrait": null,
            "sRequestId": sRequestId,
            "type": "TextBoxInputDelegate.qml"
        });
        textLog.cancelFlick();
        textLog.flick(0,-1000);
    }

    function showText(sText)
    {
        textLogModel.append({
            "textAlignment": textLog.textAlignment,
            "textContent": sText,
            "storeIntoStorageInstead": false,
            "backgroundColor": undefined !== textLog.backgroundColors  && textLog.backgroundColors.length > 0 ? textLog.backgroundColors[0] : "#ff000000",
            "textColor": undefined !== textLog.textColors  && textLog.textColors.length > 0 ? textLog.textColors[0] : "#ffffffff",
            "numButtons": "",
            "portrait": textLog.portrait,
            "sRequestId": "",
            "type": "TextBoxTextDelegate.qml"
        });
        textLog.cancelFlick();
        textLog.flick(0,-1000);
    }

    function setPortrait(sName)
    {
        if (null !== registrator.currentlyLoadedProject &&
            undefined !== registrator.currentlyLoadedProject)
        {
            if (sName === "")
            {
                textLog.portrait = null;
            }
            else
            {
                var pResource = registrator.currentlyLoadedProject.resource(sName);
                if (null !== pResource && undefined !== pResource)
                {
                    textLog.portrait = pResource;
                }
                else
                {
                    textLog.portrait = null;
                }
            }
        }
    }

    function backgroundColors()
    {
        return textLog.backgroundColors;
    }
    function setBackgroundColors(vColors)
    {
        textLog.backgroundColors = vColors;
    }

    function textColors()
    {
        return textLog.textColors;
    }
    function setTextColors(vColors)
    {
        textLog.textColors = vColors;
    }

    // accessor object for eval
    property var evalAccessor: ({
        clearTextBox: function()
        {
            textBox.clearTextBox();
        },
        showButtonPrompts: function(vsLabels, sRequestId)
        {
            signalEmitter.showButtonPrompts(vsLabels, sRequestId);
        },
        showSceneSelectionPrompts: function(vsLabels)
        {
            textBox.showButtonPrompts(vsLabels);
        },
        showInput: function(sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
        {
            signalEmitter.showInput(sStoreIntoVar, sRequestId, bStoreIntoStorageInstead);
        },
        showText: function(sText)
        {
            signalEmitter.showInput(sText);
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
        onShowButtonPrompts: {
            var vsModifiedPrompts = vsLabels;
            for (var i = 0; i < vsModifiedPrompts.length; ++i) {
                if (vsModifiedPrompts[i].startsWith("<html>") && vsModifiedPrompts[i].endsWith("</html>")) {
                    vsModifiedPrompts[i] = root.parseHtmlToJS(vsModifiedPrompts[i],  textBox.userName);
                }
            }
            textBox.showButtonPrompts(vsModifiedPrompts, sRequestId);
        }
        onShowInput: {
            textBox.showInput(sStoreIntoVar, sRequestId, bStoreIntoStorageInstead);
        }
        onShowText: {
            if (sText.startsWith("<html>") && sText.endsWith("</html>")) {
                textBox.showText(root.parseHtmlToJS(sText, textBox.userName));
            } else {
                textBox.showText(sText);
            }
            if (0 < dSkippableWaitS) {
                registrator.setSkippableWait(dSkippableWaitS);
            }
        }
        onTextAlignmentChanged: {
            textLog.textAlignment = alignment;
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
            textLog.skippable = true;
        }
        onSkippableWaitFinished: {
            textLog.skippable = false;
            signalEmitter.waitSkipped();
        }
    }

    // gui
    ListView {
        id: textLog
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height

        cacheBuffer: 0
        orientation: ListView.Vertical
        clip: true

        property var backgroundColors: [ "#ff000000" ]
        property var textColors: [ "#ffffffff" ]
        property var buttonTexts: []
        property var textAlignment: TextAlignment.AlignCenter
        property Resource portrait: null
        property bool sceneSelection: false
        property bool skippable: false
        property int iconWidth: textBox.iconWidth
        property int iconHeight: textBox.iconHeight

        function inputEditingFinished(sInput, sStoreIntoVar, sRequestId, bStoreIntoStorageInstead)
        {
            if ("" !== sStoreIntoVar)
            {
                if (bStoreIntoStorageInstead)
                {
                    root.storage.store(sStoreIntoVar, sInput);
                }
                else
                {
                    root.evaluate(sStoreIntoVar + "='" + sInput + "'");
                }
            }
            signalEmitter.showInputReturnValue(sInput, sRequestId);
        }
        function buttonPressed(iIndex, sRequestId)
        {
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
