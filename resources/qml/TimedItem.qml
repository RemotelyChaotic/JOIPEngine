import QtQuick 2.14
import QtQuick.Controls 2.14
import JOIP.core 1.1
import JOIP.ui 1.1

Rectangle {
    id: timedItem
    color: "transparent"

    property real timeMs: 0.0
    property real maxTimeMs: 0.0
    property bool showTime: true
    property bool showTimeNumber: true
    property bool running: counter.running
    property int fontSize: 20

    property alias background: backgroundLayer

    signal timeout()

    function start()
    {
        counter.updateCounter = 0;
        counter.lastDateMs = new Date().getTime();
        counter.start();
    }

    function stop()
    {
        counter.updateCounter = 0;
        counter.stop();
    }

    Timer {
        id: counter
        interval: 10
        running: false
        repeat: true

        property int updateCounter: 0
        property var lastDateMs: { return new Date().getTime(); }
        onTriggered: {
            updateCounter++;
            var newDateMs = new Date().getTime();
            var diffMs = newDateMs - lastDateMs;
            timedItem.timeMs = timedItem.timeMs - diffMs;
            lastDateMs = newDateMs;

            if (timedItem.timeMs <= 0.0)
            {
                timedItem.timeout();
                timedItem.stop();
            }

            timerDisplay.update();
        }
    }

    Rectangle {
        id: backgroundLayer
        anchors.fill: parent
        color: "transparent"

        Image {
            anchors.fill: parent

            source: Settings.styleFolderQml() + "/TimerBg.svg"
            fillMode: Image.Stretch
            smooth: true
        }
    }

    TimerDisplay {
        id: timerDisplay
        anchors.fill: parent
        fillColor: "transparent"

        primaryColor: root.style.timerDisplay.primaryColor
        secondaryColor: root.style.timerDisplay.secondaryColor
        tertiaryColor: root.style.timerDisplay.tertiaryColor

        borderWidth: root.style.timerDisplay.borderWidth
        groveWidth: root.style.timerDisplay.groveWidth

        timeMsMax: timedItem.maxTimeMs
        timeMsCurrent: timedItem.timeMs
        updateCounter: counter.updateCounter
        visibleCounter: timedItem.showTime
    }

    Rectangle {
        id: textLayer
        anchors.fill: parent
        visible: timedItem.showTimeNumber
        color: "transparent"

        Text {
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter

            text: {
                if (showTime)
                {
                    return "" + ("00" + parseInt(timedItem.timeMs / 60000)).substr(-2) +
                            ":" + ("00" + parseInt((timedItem.timeMs / 1000) % 60, 10)).substr(-2)
                }
                else
                {
                    return "?";
                }
            }
            font.family: Settings.font
            font.pointSize: timedItem.fontSize
            antialiasing: true

            color: "black"
            style: Text.Outline
            styleColor: "white"
        }
    }
}
