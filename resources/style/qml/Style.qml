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
        property color primaryColor: "#760076"
        property color secondaryColor: "#b90064"
        property color tertiaryColor: "#ec00ec"

        property int borderWidth: 2
        property int groveWidth: 10
    }
    property QtObject metronomeDisplay: QtObject {
        property color centerColor: "#99ec00ec"
        property color barColor: "#99b90064"
        property color ticksColor: "silver"
        property color rippleColor: "#ffb90064"
    }
}
