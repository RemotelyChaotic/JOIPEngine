import QtQuick 2.14
import JOIP.db 1.1

Item {
    id: registrator

    property Project currentlyLoadedProject: root.currentlyLoadedProject

    function componentLoaded()
    {
        root.numReadyComponents += 1;
        root.componentsRegistered[root.numReadyComponents] = registrator;
    }

    signal skippableWait(int iTimeS)
    signal skippableWaitFinished()
    function registerTextBox(item)
    {
        root.registeredTextBox = item;
    }

    function registerMediaPlayer(item)
    {
        root.registeredMediaPlayer = item;
    }
}
