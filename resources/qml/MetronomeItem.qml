import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1
import JOIP.ui 1.1

Rectangle {
    id: metronomeItem
    color: "transparent"

    property string beatResource: ""
    property int bpm: 60
    property var pattern: [ 1 ]
    property bool running: counter.running

    function start()
    {
        counter.lastDateMs = new Date().getTime();
        counter.currentInterval = 0.0;
        counter.currentPatternIndex = 0;
        counter.start();
    }

    function resume()
    {
        if (!counter.running)
        {
            counter.lastDateMs = new Date().getTime();
            counter.start();
        }
    }

    function pause()
    {
        if (counter.running)
        {
            counter.stop();
        }
    }

    function stop()
    {
        counter.stop();
        counter.currentInterval = 0.0;
        counter.currentPatternIndex = 0;
        metronomeDisplay.clear();
    }

    Timer {
        id: counter
        interval: 10
        running: false
        repeat: true

        property real currentInterval: 0.0
        property int currentPatternIndex: 0
        property real bpmInterval: 60 / metronomeItem.bpm * 1000
        property var lastDateMs: { return new Date().getTime(); }
        onTriggered: {
            var newDateMs = new Date().getTime();
            var diffMs = newDateMs - lastDateMs;
            lastDateMs = newDateMs;

            currentInterval = currentInterval + diffMs;
            if (metronomeItem.pattern.length > currentPatternIndex && -1 < currentPatternIndex)
            {
                if (currentInterval >= metronomeItem.pattern[currentPatternIndex] * bpmInterval)
                {
                    currentInterval = currentInterval - metronomeItem.pattern[currentPatternIndex] * bpmInterval;
                    metronomeDisplay.spawnNewMetronomeTicks();

                    currentPatternIndex = currentPatternIndex + 1;
                    if (metronomeItem.pattern.length <= currentPatternIndex)
                    {
                        currentPatternIndex = 0;
                    }
                }
            }
            else
            {
                if (currentInterval >= bpmInterval)
                {
                    currentInterval = currentInterval - bpmInterval;
                    metronomeDisplay.spawnNewMetronomeTicks();
                }
                currentPatternIndex = 0;
            }

            metronomeDisplay.update(diffMs);
        }
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

        beatResource: metronomeItem.beatResource
        tickColor: root.style.metronomeDisplay.ticksColor

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
            smooth: true
            antialiasing: true
            mipmap: true
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


}
