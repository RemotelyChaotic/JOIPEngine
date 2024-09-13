import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1
import JOIP.ui 1.1

Rectangle {
    id: metronomeItem
    color: "transparent"

    property string userName: "metronome"
    onUserNameChanged: {
        metronomeDisplay.registerUi(metronomeItem.userName);
    }

    property string beatResources: []
    property int bpm: 60
    property var pattern: [ 1 ]
    property bool running: metronomeDisplay.running
    property bool muted: false
    property double volume: 1.0

    function start()
    {
        metronomeDisplay.start();
    }

    function resume()
    {
        metronomeDisplay.resume();
    }

    function pause()
    {
        metronomeDisplay.pause();
    }

    function stop()
    {
        metronomeDisplay.stop();
    }

    Rectangle {
        id: lineLayer
        anchors.centerIn: parent
        width: parent.width > parent.height ? parent.width : 5
        height: parent.width > parent.height ? 5 : parent.height

        color: root.style.metronomeDisplay.barColor
        radius: parent.width > parent.height ? (height / 2) : (width / 2)
    }

    MetronomeDisplay {
        id: metronomeDisplay
        anchors.centerIn: parent
        width: parent.width
        height: parent.height

        property bool running: false

        beatResources: metronomeItem.beatResources
        bpm: metronomeItem.bpm
        muted: metronomeItem.muted
        pattern: metronomeItem.pattern
        tickColor: root.style.metronomeDisplay.ticksColor
        metCmdMode: root.currentlyLoadedProject.metCmdMode
        volume: metronomeItem.volume

        Behavior on volume {
            animation: NumberAnimation {
                id: audiofade
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }

        property int currentRipple: 0
        onTickReachedCenter: {
            switch(currentRipple)
            {
            case 0: gradient1.start(); break;
            case 1: gradient2.start(); break;
            case 2: gradient3.start(); break;
            case 3: gradient4.start(); break;
            }
            currentRipple++
            if (4 == currentRipple) { currentRipple = 0; }
        }
        onMetronomeStateChanged: {
            switch (state)
            {
            case MetronomeDisplay.Started: running = true; break;
            case MetronomeDisplay.Paused: running = true; break;
            case MetronomeDisplay.Resumed: running = true; break;
            case MetronomeDisplay.Stopped: running = false; break;
            }
        }
    }

    Rectangle {
        id: centerLayer
        anchors.centerIn: parent
        height: parent.width > parent.height ? parent.height : parent.width
        width: parent.width > parent.height ? parent.height : parent.width
        color: "transparent"

        smooth: Settings.playerImageSmooth
        antialiasing: Settings.playerAntialiasing

        Rectangle {
            anchors.centerIn: parent
            width: metronomeItem.width > metronomeItem.height ? 5 : (parent.width / 2)
            height: metronomeItem.width > metronomeItem.height ? (parent.height / 2) : 5

            color: root.style.metronomeDisplay.centerColor
            radius: metronomeItem.width > metronomeItem.height ? (width / 2) : (height / 2)
        }

        Image {
            anchors.centerIn: parent
            width: parent.width / 4
            height: parent.height / 4

            source: Settings.styleFolderQml() + "/TimerBg.svg"
            fillMode: Image.Stretch
            smooth: Settings.playerImageSmooth
            antialiasing: Settings.playerAntialiasing
            mipmap: Settings.playerImageMipMap
        }

        RadialGradient {
            anchors.centerIn: parent
            width: parent.width
            height: parent.height

            source: Rectangle {
                color: "white"
                anchors.centerIn: parent
                width: centerLayer.width
                height: centerLayer.height
                radius: width / 2
            }

            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00000000" }
                GradientStop {
                    position: 0.05
                    color: "#00000000"
                }
                GradientStop {
                    position: 0.1
                    color: root.style.metronomeDisplay.rippleColor
                }
                GradientStop {
                    position: 0.15
                    color: "#00000000"
                }
                GradientStop { position: 1.0; color: "#00000000" }
            }
        }

        MetronomeRippleItem {
            id: gradient1
            anchors.centerIn: parent
            width: parent.width
            height: parent.height

            centerLayerWidth: centerLayer.width
            centerLayerHeight: centerLayer.height
        }
        MetronomeRippleItem {
            id: gradient2
            anchors.centerIn: parent
            width: parent.width
            height: parent.height

            centerLayerWidth: centerLayer.width
            centerLayerHeight: centerLayer.height
        }
        MetronomeRippleItem {
            id: gradient3
            anchors.centerIn: parent
            width: parent.width
            height: parent.height

            centerLayerWidth: centerLayer.width
            centerLayerHeight: centerLayer.height
        }
        MetronomeRippleItem {
            id: gradient4
            anchors.centerIn: parent
            width: parent.width
            height: parent.height

            centerLayerWidth: centerLayer.width
            centerLayerHeight: centerLayer.height
        }
    }

    Component.onCompleted: {
        metronomeDisplay.registerUi(metronomeItem.userName);
    }
}
