import QtQuick 2.14
import QtQuick.Controls 2.14
import JOIP.core 1.5
import JOIP.ui 1.1

Rectangle {
    id: timedItem
    color: "transparent"

    property real timeMs: 0.0
    property real maxTimeMs: 0.0
    property bool showClock: true
    property bool showTime: true
    property bool showTimeNumber: true
    property bool running: counter.running
    property int fontSize: 20
    property string font: Settings.font

    property alias background: backgroundLayer.data

    signal timeout()

    function start()
    {
        counter.updateCounter = 0;
        counter.lastDateMs = new Date().getTime();
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
        counter.updateCounter = 0;
        timedItem.timeout();
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

        data: Image {
            anchors.fill: parent
            source: Settings.styleFolderQml() + "/TimerBg.svg"
            fillMode: Image.Stretch
            smooth: Settings.playerImageSmooth
            antialiasing: Settings.playerAntialiasing
            mipmap: Settings.playerImageMipMap
        }
    }

    TimerDisplay {
        id: timerDisplay
        anchors.fill: parent
        fillColor: "transparent"

        visible: timedItem.showClock
        smooth: Settings.playerImageSmooth
        antialiasing: Settings.playerAntialiasing

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
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            text: {
                var sText;
                if (showTime)
                {
                    sText = "" + ("00" + parseInt(timedItem.timeMs / 60000)).substr(-2) +
                            ":" + ("00" + parseInt((timedItem.timeMs / 1000) % 60, 10)).substr(-2)
                }
                else
                {
                    sText = "?";
                }
                return sText;
            }

            font.family: timedItem.font
            font.pointSize: timedItem.fontSize
            font.hintingPreference: Font.PreferNoHinting

            renderType: Text.NativeRendering
            color: "black"
            style: Text.Outline
            styleColor: "white"

        }
    }
}
