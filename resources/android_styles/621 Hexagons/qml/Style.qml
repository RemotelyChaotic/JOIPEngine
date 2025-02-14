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
        property color primaryColor: "#031131"
        property color secondaryColor: "#00759f"
        property color tertiaryColor: "#152f56"

        property int borderWidth: 2
        property int groveWidth: 10
    }
    property QtObject metronomeDisplay: QtObject {
        property color centerColor: "#99152f56"
        property color barColor: "#99152f56"
        property color ticksColor: "white"
        property color rippleColor: "#ff00759f"
    }
    property QtObject achievementDisplay: QtObject {
        property color backgroundColor: "#ff152f56"
        property color borderColor: "#ff00759f"
        property color textColor: "#ffb4c7db"
        property int borderWidth: 2
        property int borderRadius: 5
    }
}
