import QtQuick 2.14

QtObject {
    property QtObject controllButtonDisplay: QtObject {
        property var topOffset: 50
        property var rightOffset: 50
        property var height: 64
    }
    property QtObject backgroundDisplay: QtObject {
        property var horizontalTileMode: BorderImage.Stretch
        property var verticalTileMode: BorderImage.Stretch
    }
    property QtObject timerDisplay: QtObject {
        property color primaryColor: "#170738"
        property color secondaryColor: "#25193d"
        property color tertiaryColor: "#372a43"

        property int borderWidth: 2
        property int groveWidth: 10
    }
    property QtObject metronomeDisplay: QtObject {
        property color centerColor: "#99372a43"
        property color barColor: "#9925193d"
        property color ticksColor: "white"
        property color rippleColor: "#ff25193d"
    }
}
