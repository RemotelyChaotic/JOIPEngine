import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.5
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: timer
    color: "transparent"
    property string userName: "timer"

    // accessor object for eval
    property var evalAccessor: ({
        hideTimer: function()
        {
            signalEmitter.hideTimer();
        },
        setTime: function(dTimeS)
        {
            signalEmitter.setTime(dTimeS);
        },
        setTimeVisible: function(bVisible)
        {
            signalEmitter.setTimeVisible(bVisible);
        },
        showTimer: function()
        {
            signalEmitter.showTimer();
        },
        startTimer: function()
        {
            signalEmitter.startTimer();
        },
        stopTimer: function()
        {
            signalEmitter.stopTimer();
        }
    });

    TimerSignalEmitter {
        id: signalEmitter

        onHideTimer: {
            timerDisplay.opacity = 0.0;
        }
        onSetTime: {
            timerDisplay.timeMs = dTimeS * 1000.0
            timerDisplay.maxTimeMs = timerDisplay.timeMs;
        }
        onSetTimeVisible: {
            timerDisplay.showTime = bVisible;
        }
        onShowTimer: {
            timerDisplay.opacity = 1.0;
        }
        onStartTimer: {
            timerDisplay.bStarted = true;
            timerDisplay.start();
        }
        onStopTimer: {
            timerDisplay.stop();
            timerDisplay.bStarted = false;
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
        name: timer.userName
    }

    // UI
    TimedItem {
        id: timerDisplay
        anchors.centerIn: parent
        width: parent.width - 10
        height: parent.width - 10

        showTimeNumber: true
        font: registrator.playerFont

        property bool bStarted: false

        onTimeout: {
            signalEmitter.timerFinished();
            bStarted = false;
            opacity = 0.0;
        }

        opacity: 0.0
        Behavior on opacity {
            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
    }

    Component.onCompleted: {
        registrator.registerNewScriptComponent(signalEmitter);
        registrator.registerNewUiComponent(evalAccessor);
    }

    // handle interrupt
    Connections {
        target: ScriptRunner
        onRunningChanged: {
            if (timerDisplay.bStarted) {
                if (!bRunning)
                {
                    timerDisplay.pause();
                }
                else
                {
                    timerDisplay.resume();
                }
            }
        }
    }
}
