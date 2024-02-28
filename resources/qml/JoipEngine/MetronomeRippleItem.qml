import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14

RadialGradient {
    id: gradient
    anchors.centerIn: parent
    width: parent.width
    height: parent.height

    property int centerLayerWidth: width
    property int centerLayerHeight: height

    function start()
    {
        beginAnim.start();
        rippleAnim.start();
        endAnim.start();
    }

    source: Rectangle {
        color: "white"
        anchors.centerIn: parent
        width: gradient.centerLayerWidth
        height: gradient.centerLayerHeight
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
