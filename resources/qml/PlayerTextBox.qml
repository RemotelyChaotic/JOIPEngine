import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

import "qrc:/xmldom/dom-parser.mjs" as DOMParser;

Rectangle {
    id: textBox
    color: "transparent"
    property string userName: "textBox"
    property bool mainTextBox: false

    function clearTextBox()
    {
        textLogModel.clear();
        textLogModel.append({
            "textAlignment": "",
            "textContent": "",
            "backgroundColor": "",
            "textColor": "",
            "numButtons": "",
            "portrait": null,
            "type": "TextBoxSpacerDelegate.qml"
        });
        textLog.cancelFlick();
        textLog.flick(0,-1000);
    }

    function showButtonPrompts(vsLabels)
    {
        textLog.buttonTexts = vsLabels;
        textLogModel.append({
            "textAlignment": "",
            "textContent": "",
            "backgroundColor": "",
            "textColor": "",
            "numButtons": vsLabels.length,
            "portrait": null,
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

    function showInput(sStoreIntoVar)
    {
        textLogModel.append({
            "textAlignment": "",
            "textContent": sStoreIntoVar,
            "backgroundColor": undefined !== textLog.backgroundColors  && textLog.backgroundColors.length > 0 ? textLog.backgroundColors[0] : "#ff000000",
            "textColor": undefined !== textLog.textColors  && textLog.textColors.length > 0 ? textLog.textColors[0] : "#ffffffff",
            "numButtons": "",
            "portrait": null,
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
            "backgroundColor": undefined !== textLog.backgroundColors  && textLog.backgroundColors.length > 0 ? textLog.backgroundColors[0] : "#ff000000",
            "textColor": undefined !== textLog.textColors  && textLog.textColors.length > 0 ? textLog.textColors[0] : "#ffffffff",
            "numButtons": "",
            "portrait": textLog.portrait,
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
                    vsModifiedPrompts[i] = parseHtmlToJS(vsModifiedPrompts[i]);
                }
            }
            textBox.showButtonPrompts(vsModifiedPrompts);
        }
        onShowInput: {
            textBox.showInput(sStoreIntoVar);
        }
        onShowText: {
            if (sText.startsWith("<html>") && sText.endsWith("</html>")) {
                textBox.showText(parseHtmlToJS(sText));
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

        function inputEditingFinished(sInput, sStoreIntoVar)
        {
            if ("" !== sStoreIntoVar)
            {
                root.storage.store(sStoreIntoVar, sInput);
            }
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

    //------------------------------------------------------------------------------------
    // variables
    // from openeos:
    // Convert HTML string to in-line javascript string expression
    // Replace ...<eval>expression</eval>... with "..." + (isolated eval expression) + "..."
    function parseHtmlToJS(string) {
      if (typeof string !== 'string') return JSON.stringify('');
      const result = [];
      const parser = new DOMParser.DOMParser();
      const doc = parser
        .parseFromString(string, 'text/html')
        .getElementsByTagName('body')[0];
      if (null == doc) return string;
      const evs = doc.getElementsByTagName('eval');
      if (null == evs) return string;
      let docstring = doc.innerHTML;
      if (null == docstring) return string;
      for (var iEv = 0; iEv < evs.length; ++iEv) {
        const ev = evs.item(iEv);
        if (null == ev) continue;
        const evHtml = ev.outerHTML;
        const i = docstring.indexOf(evHtml);
        const beforeEv = docstring.slice(0, i);
        const afterEv = docstring.slice(i + evHtml.length, docstring.length);
        if (beforeEv.length) {
          result.push(JSON.stringify(beforeEv));
        }
        const evExpression = QtApp.decodeHTML(ev.innerHTML).trim();
        if (evExpression.length) {
          result.push(root.wrap(evExpression, 'e.toString()', 'Say/Text <eval>'));
        }
        docstring = afterEv;
      }
      if (docstring.length) {
        result.push(JSON.stringify(docstring));
      }
      if (!result.length) return JSON.stringify('');
      return result.join(' + ');
    }
}
