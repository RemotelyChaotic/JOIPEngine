import QtQuick 2.14
import JOIP.core 1.5
import JOIP.db 1.1

Item {
    id: registrator

    property Project currentlyLoadedProject: root.currentlyLoadedProject
    property string playerFont: {
        null == currentlyLoadedProject ? Settings.font : currentlyLoadedProject.font
    }

    function componentLoaded()
    {
        root.numReadyComponents += 1;
        root.componentsRegistered[root.numReadyComponents] = registrator;
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

    function registerTextBox(item)
    {
        root.registeredTextBox = item;
    }

    signal soundFinished(string sResource)
    signal volumeSet(string sResource, real dValue)
    signal audioTryToCallWith(string sResource, string fn, var data, var fnNotFound)
    signal playAudioRequested(string sResource, string sId, int iLoops, int iStartAt, int iEndAt)

    function setVolume(sResource, dValue)
    {
        for (var i = 0; root.componentsRegistered.length > i; ++i)
        {
            if (null !== root.componentsRegistered[i] && undefined !== root.componentsRegistered[i])
            {
                root.componentsRegistered[i].volumeSet(sResource, dValue);
            }
        }
    }
    function tryToCallAudio(sResource, fn, data, fnNotFound)
    {
        for (var i = 0; root.componentsRegistered.length > i; ++i)
        {
            if (null !== root.componentsRegistered[i] && undefined !== root.componentsRegistered[i])
            {
                root.componentsRegistered[i].audioTryToCallWith(sResource, fn, data, fnNotFound);
            }
        }
    }
    function playAudio(sName, sId, iLoops, iStartAt, iEndAt)
    {
        for (var i = 0; root.componentsRegistered.length > i; ++i)
        {
            if (null !== root.componentsRegistered[i] && undefined !== root.componentsRegistered[i])
            {
                root.componentsRegistered[i].playAudioRequested(sName, sId, iLoops, iStartAt, iEndAt);
            }
        }
    }
}
