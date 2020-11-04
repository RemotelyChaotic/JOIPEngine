import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: metronome
    color: "transparent"
    property string userName: "metronome"

    MetronomeSignalEmitter {
        id: signalEmitter

        onStart: {
            metronomeDisplay.start();
            metronomeDisplay.opacity = 1.0;
            metronomeDisplay.requestedRun = true;
        }
        onSetBpm: {
            metronomeDisplay.bpm = iBpm;
        }
        onSetBeatResource: {
            if (sResource === "")
            {
                metronomeDisplay.beatResource = "";
            }
            else
            {
                if (null !== registrator.currentlyLoadedProject &&
                    undefined !== registrator.currentlyLoadedProject)
                {
                    var pResource = registrator.currentlyLoadedProject.resource(sResource);
                    if (null !== pResource && undefined !== pResource)
                    {
                        if (pResource.type === Resource.Sound)
                        {
                            metronomeDisplay.beatResource = pResource.path;
                        }
                    }
                }
            }
        }
        onSetPattern: {
            metronomeDisplay.pattern = vdPattern;
        }
        onStop: {
            metronomeDisplay.opacity = 0.0;
            metronomeDisplay.stop();
            metronomeDisplay.requestedRun = false;
        }
    }

    PlayerComponentRegistrator {
        id: registrator
    }

    MetronomeItem {
        id: metronomeDisplay
        anchors.centerIn: parent
        width: parent.width - 10
        height: parent.height - 10

        property bool requestedRun: false

        opacity: 0.0
        Behavior on opacity {
            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
        }
    }

    // handle interrupt
    Connections {
        target: ScriptRunner
        onRunningChanged: {
            if (!bRunning)
            {
                metronomeDisplay.pause();
            }
            else
            {
                if (metronomeDisplay.requestedRun)
                {
                    metronomeDisplay.resume();
                }
            }
        }
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        registrator.componentLoaded();
    }
}
