import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: timer
    color: "transparent"
    property string userName: "timer"

    TimerSignalEmitter {
        id: signalEmitter

        onHideTimer: {
            timerDisplay.opacity = 0.0;
        }
        onSetTime: {
            timerDisplay.timeMs = iTimeS * 1000
            timerDisplay.maxTimeMs = timerDisplay.timeMs;
        }
        onSetTimeVisible: {
            timerDisplay.showTime = bVisible;
        }
        onShowTimer: {
            timerDisplay.opacity = 1.0;
        }
        onStartTimer: {
            timerDisplay.start();
        }
        onStopTimer: {
            timerDisplay.stop();
        }
        onWaitForTimer: {
            if (!timerDisplay.running)
            {
                timerFinished();
            }
        }
    }

    PlayerComponentRegistrator {
        id: registrator
    }

    // UI
    TimedItem {
        id: timerDisplay
        anchors.centerIn: parent
        width: parent.width - 10
        height: parent.width - 10

        showTimeNumber: true

        onTimeout: {
            signalEmitter.timerFinished();
            opacity = 0.0;
        }

        opacity: 0.0
        Behavior on opacity {
            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
        }
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        registrator.componentLoaded();
    }
}
