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

    function registerMediaPlayer(item)
    {
        root.registeredMediaPlayer = item;
    }
}
