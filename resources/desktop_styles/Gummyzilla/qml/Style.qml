import QtQuick 2.14

QtObject {
    property QtObject controllButtonDisplay: QtObject {
        property var topOffset: 50
        property var rightOffset: 50
        property var height: 64
    }
    property QtObject backgroundDisplay: QtObject {
        property var horizontalTileMode: BorderImage.Round
        property var verticalTileMode: BorderImage.Round
    }
    property QtObject timerDisplay: QtObject {
        property color primaryColor: "#2a2a2a"
        property color secondaryColor: "#1bbc9d"
        property color tertiaryColor: "#1bbc9d"

        property int borderWidth: 2
        property int groveWidth: 10
    }
    property QtObject metronomeDisplay: QtObject {
        property color centerColor: "#992a2a2a"
        property color barColor: "#992a2a2a"
        property color ticksColor: "white"
        property color rippleColor: "#ff1bbc9d"
    }
}
