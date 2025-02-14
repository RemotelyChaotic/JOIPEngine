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
        property color primaryColor: "#131c28"
        property color secondaryColor: "#488de6"
        property color tertiaryColor: "#344561"

        property int borderWidth: 2
        property int groveWidth: 10
    }
    property QtObject metronomeDisplay: QtObject {
        property color centerColor: "#99344561"
        property color barColor: "#99488de6"
        property color ticksColor: "white"
        property color rippleColor: "#ff488de6"
    }
    property QtObject achievementDisplay: QtObject {
        property color backgroundColor: "#ff24344D"
        property color borderColor: "#ff344561"
        property color textColor: "white"
        property int borderWidth: 1
        property int borderRadius: 0
    }
}
