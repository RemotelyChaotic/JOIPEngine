import QtQuick 2.14
import QtQuick.Templates 2.14
import QtGraphicalEffects 1.14

Button {
    id: control
    contentItem: Text {
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        color: "transparent"
        Rectangle {
            id: bg
            anchors.fill: parent
            radius: 10
        }
        RadialGradient {
            anchors.fill: parent
            source: bg
            verticalOffset: -parent.height / 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: control.enabled ? (control.down ? "#dd00dd" : (control.hovered ? "#aa00aa" : "#aa00aa")) : "#262626" }
                GradientStop { position: 0.5; color: control.enabled ? (control.down ? "#aa00aa" : (control.hovered ? "#b900b9" : "#760076")) : "#262626" }
                GradientStop { position: 1.0; color: control.enabled ? (control.down ? "#aa00aa" : (control.hovered ? "#b900b9" : "#760076")) : "#262626" }
            }
        }
        Rectangle {
            id: border
            anchors.fill: parent
            border.color: "silver"
            border.width: 2
            color: "transparent"
            radius: 10
        }
    }
    //color:
}

