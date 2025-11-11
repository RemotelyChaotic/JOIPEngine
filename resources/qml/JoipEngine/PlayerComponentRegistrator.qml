import QtQuick 2.14
import JOIP.core 1.5
import JOIP.db 1.1
import JOIP.script 1.1

Item {
    id: registrator
    property string name: "customComponent"

    property Project currentlyLoadedProject: root.currentlyLoadedProject
    property string playerFont: {
        null == currentlyLoadedProject ? Settings.font : currentlyLoadedProject.font
    }

    signal skippableWait(int iTimeS)
    signal skippableWaitFinished()
    function setSkippableWait(iTimeS)
    {
        if (0 !== iTimeS)
        {
            skippableWait(iTimeS);
        }
        else
        {
            skippableWaitFinished();
        }
    }

    function registerNewScriptComponent(comp)
    {
        ScriptRunner.registerNewComponent(name, comp);
    }
    function registerNewUiComponent(comp, vsOptAliases)
    {
        root.registerUIComponent(name, comp);
        if (null != vsOptAliases) {
            if (typeof vsOptAliases === "string") {
                root.registerUIComponent(vsOptAliases, comp);
            } else /*Array assumed*/ {
                for (var i = 0; vsOptAliases.length > i; ++i) {
                    root.registerUIComponent(vsOptAliases[i], comp);
                }
            }
        }
    }

    function registerTextBox(item)
    {
        root.registeredTextBox = item;
    }
    function unRegisterTextBox()
    {
        root.registeredTextBox = null;
    }

    signal soundFinished(string sResource)
    signal volumeSet(string sResource, real dValue)
    signal audioTryToCallWith(string sResource, string fn, var data, var fnNotFound)
    signal playAudioRequested(string sResource, string sId, int iLoops, int iStartAt, int iEndAt)

    function setVolume(sResource, dValue)
    {
        for (var i = 0; root.componentsRegistered.length > i; ++i)
        {
            if (null !== root.componentsRegistered[i].obj && undefined !== root.componentsRegistered[i].obj)
            {
                root.componentsRegistered[i].obj.volumeSet(sResource, dValue);
            }
        }
    }
    function tryToCallAudio(sResource, fn, data, fnNotFound)
    {
        for (var i = 0; root.componentsRegistered.length > i; ++i)
        {
            if (null !== root.componentsRegistered[i].obj && undefined !== root.componentsRegistered[i].obj)
            {
                root.componentsRegistered[i].obj.audioTryToCallWith(sResource, fn, data, fnNotFound);
            }
        }
    }
    function playAudio(sName, sId, iLoops, iStartAt, iEndAt)
    {
        for (var i = 0; root.componentsRegistered.length > i; ++i)
        {
            if (null !== root.componentsRegistered[i].obj && undefined !== root.componentsRegistered[i].obj)
            {
                root.componentsRegistered[i].obj.playAudioRequested(sName, sId, iLoops, iStartAt, iEndAt);
            }
        }
    }

    Component.onCompleted: {
        for (var i = 0; root.componentsRegistered.length > i; ++i) {
            if (root.componentsRegistered[i].name === name) {
                root.componentsRegistered.splice(i, 1);
                break;
            }
        }

        root.componentsRegistered.push({"name": name, "obj": registrator});
    }
    Component.onDestruction: {
        for (var i = 0; root.componentsRegistered.length > i; ++i) {
            if (root.componentsRegistered[i].name === name) {
                root.componentsRegistered.splice(i, 1);
                break;
            }
        }
    }
}
