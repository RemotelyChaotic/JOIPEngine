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

    property string beatResource: ""
    property int bpm: 60
    property var pattern: [ 1 ]
    property bool running: metronomeDisplay.running
    property bool muted: false
    property double volume: 1.0

    function start()
    {
        metronomeDisplay.start();
        metronomeDisplay.running = true;
    }

    function resume()
    {
        if (!metronomeDisplay.running)
        {
            metronomeDisplay.resume();
            metronomeDisplay.running = true;
        }
    }

    function pause()
    {
        if (metronomeDisplay.running)
        {
            metronomeDisplay.pause();
            metronomeDisplay.running = false;
        }
    }

    function stop()
    {
        metronomeDisplay.stop();
        metronomeDisplay.running = false;
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

        beatResource: metronomeItem.beatResource
        bpm: metronomeItem.bpm
        muted: metronomeItem.muted
        pattern: metronomeItem.pattern
        tickColor: root.style.metronomeDisplay.ticksColor
        volume: metronomeItem.volume

        Behavior on volume {
            animation: NumberAnimation {
                id: audiofade
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }

        onTickReachedCenter: {
            beginAnim.start();
            rippleAnim.start();
            endAnim.start();
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

        RadialGradient {
            id: gradient
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
                    id: rippleBegin
                    position: 0.05
                    color: "#00000000"

                    NumberAnimation on position {
                        id: beginAnim
                        duration: 500
                        to: 0.85
                        onRunningChanged: {
                            if (!running) {
                                rippleBegin.position = 0.05;
                            }
                        }
                    }
                }
                GradientStop {
                    id: ripplePos
                    position: 0.1
                    color: root.style.metronomeDisplay.rippleColor

                    NumberAnimation on position {
                        id: rippleAnim
                        duration: 500
                        to: 0.9
                        onRunningChanged: {
                            if (!running) {
                                ripplePos.position = 0.1;
                            }
                        }
                    }
                }
                GradientStop {
                    id: rippleEnd
                    position: 0.15
                    color: "#00000000"

                    NumberAnimation on position {
                        id: endAnim
                        duration: 500
                        to: 0.95
                        onRunningChanged: {
                            if (!running) {
                                rippleEnd.position = 0.15;
                            }
                        }

                    }
                }
                GradientStop { position: 1.0; color: "#00000000" }
            }
        }
    }

    Component.onCompleted: {
        metronomeDisplay.registerUi(metronomeItem.userName);
    }
}
