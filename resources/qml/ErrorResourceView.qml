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

    property real fontSize: 100
    property real glowRadius: 10

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

    FontMetrics {
        id: fontMetrics
        font.family: text404.font.family
        font.pixelSize: fontSize
    }

    Text {
        id: text404
        text: "404"
        anchors.fill: parent
        color: "black"
        font.family: Settings.font
        font.pixelSize: {
            var iWidth = fontMetrics.boundingRect(text404.text).width;
            var dRatio = 1.0;
            if (text404Rect.parent.width < iWidth + text404Rect.glowRadius*2+5)
            {
                dRatio = text404Rect.parent.width / (iWidth + text404Rect.glowRadius*2+5);
            }
            return text404Rect.fontSize * dRatio;
        }

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        layer.enabled: true
        layer.effect: Glow {
            radius: text404Rect.glowRadius
            samples: 17
            spread: 0.5
            color: "white"
            transparentBorder: false
        }
    }
}
