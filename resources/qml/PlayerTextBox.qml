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

    function clearTextBox()
    {
        textLogModel.clear();
        textLogModel.append({
            "textContent": "",
            "backgroundColor": "",
            "textColor": "",
            "numButtons": "",
            "type": "TextBoxSpacerDelegate.qml"
        });
        textLog.flick(0,-1000);
    }

    function showButtonPrompts(vsLabels)
    {
        textLog.buttonTexts = vsLabels;
        textLogModel.append({
            "textContent": "",
            "backgroundColor": "",
            "textColor": "",
            "numButtons": vsLabels.length,
            "type": "TextBoxButtonsDelegate.qml"
        });
        textLog.flick(0,-1000);
    }

    signal sceneSelectionRetVal(int iValue)
    function showSceneSelectionPrompts(vsLabels)
    {
        textLog.sceneSelection = true;
        showButtonPrompts(vsLabels);
    }

    function showInput()
    {
        textLogModel.append({
            "textContent": "",
            "backgroundColor": undefined !== textLog.backgroundColors  && textLog.backgroundColors.length > 0 ? textLog.backgroundColors[0] : "#ff000000",
            "textColor": undefined !== textLog.textColors  && textLog.textColors.length > 0 ? textLog.textColors[0] : "#ffffffff",
            "numButtons": "",
            "type": "TextBoxInputDelegate.qml"
        });
        textLog.flick(0,-1000);
    }

    function showText(sText)
    {
        textLogModel.append({
            "textContent": sText,
            "backgroundColor": undefined !== textLog.backgroundColors  && textLog.backgroundColors.length > 0 ? textLog.backgroundColors[0] : "#ff000000",
            "textColor": undefined !== textLog.textColors  && textLog.textColors.length > 0 ? textLog.textColors[0] : "#ffffffff",
            "numButtons": "",
            "type": "TextBoxTextDelegate.qml"
        });
        textLog.flick(0,-1000);
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

    // signal handling from script
    TextBoxSignalEmitter {
        id: signalEmitter

        onClearText: {
            textBox.clearTextBox();
        }
        onShowButtonPrompts: {
            textBox.showButtonPrompts(vsLabels);
        }
        onShowInput: {
            textBox.showInput();
        }
        onShowText: {
            textBox.showText(sText);
        }
        onTextBackgroundColorsChanged: {
            setBackgroundColors(vColors);
        }
        onTextColorsChanged: {
            setTextColors(vColors);
        }
    }

    PlayerComponentRegistrator {
        id: registrator

        onSkippableWait: {
            textLog.skippable = true;
        }
        onSkippableWaitFinished: {
            textLog.skippable = false;
        }
    }

    // gui
    ListView {
        id: textLog
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height

        orientation: ListView.Vertical
        clip: true

        property var backgroundColors: [ "#ff000000" ]
        property var textColors: [ "#ffffffff" ]
        property var buttonTexts: []
        property bool sceneSelection: false
        property bool skippable: false

        function inputEditingFinished(sInput)
        {
            signalEmitter.showInputReturnValue(sInput);
        }
        function buttonPressed(iIndex)
        {
            if (!sceneSelection)
            {
                signalEmitter.showButtonReturnValue(iIndex);
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
    }
}
