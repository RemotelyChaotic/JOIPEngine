import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Shapes 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.5
import JOIP.db 1.1

Rectangle {
    id: card
    anchors.centerIn: parent
    width: parent.width
    height: parent.height

    property Resource resource: null
    property var selectionColor: "white"
    property bool isSelected: false
    property bool isHovered: false
    property string errorText: ""

    color: "transparent"
    layer.enabled: card.isSelected
    layer.effect: Glow {
        radius: 8
        samples: 17
        spread: 0.5
        color: card.selectionColor
        transparentBorder: false
    }

    AnimatedImage {
        id: loadingAnimation
        anchors.fill: parent
        visible: resource.state === Resource.Loading
        source: "qrc:/resources/gif/spinner_transparent.gif"
        fillMode: Image.Pad
    }

    ImageResourceView {
        id: resource
        anchors.centerIn: parent
        width: parent.width - 10
        height: parent.height - 10
        resource: card.resource

        // Gloss
        Rectangle {
            id: glossRect
            x: (resource.width - resource.paintedWidth) / 2
            y: (resource.height - resource.paintedHeight) / 2
            width: resource.paintedWidth
            height: resource.paintedHeight
            color: "transparent"

            LinearGradient {
                anchors.fill: parent
                start: Qt.point(parent.width, 0)
                end: Qt.point(parent.width * 2 / 3, parent.height / 2)
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: card.isSelected ? "#DDF0F0F0" : "#AAF0F0F0"
                    }
                    GradientStop {
                        position: 1.0
                        color: "#00F0F0F0"
                    }
                }
            }
        }
    }

    Desaturate {
        id: desaturateEffect
        anchors.fill: resource
        source: resource
        desaturation: 0.0
    }

    ErrorResourceView {
        id: errorResource
        visible: resource.state === Resource.Null || resource.state === Resource.Error
        fontSize: card.isHovered ? 100 : 90
        text: card.errorText
    }
}
