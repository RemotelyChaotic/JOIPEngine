import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.12
import JOIP.core 1.1

Rectangle {
    id: text404Rect
    anchors.centerIn: parent
    visible: true
    width: visible ? parent.width : 0
    height: visible ? parent.height : 0

    property alias fontSize: text404.font.pixelSize

    color: "transparent"

    Behavior on height {
        animation: ParallelAnimation {
            NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
        }
    }
    Behavior on width {
        animation: ParallelAnimation {
            NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
        }
    }

    Text {
        id: text404
        text: "404"
        anchors.fill: parent
        color: "black"
        font.family: Settings.font
        font.pixelSize: 100
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        layer.enabled: true
        layer.effect: Glow {
            radius: 10
            samples: 17
            spread: 0.5
            color: "white"
            transparentBorder: false
        }
    }
}
