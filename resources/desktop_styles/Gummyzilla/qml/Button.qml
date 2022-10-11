import QtQuick 2.14
import QtQuick.Templates 2.14

Button {
    id: control
    contentItem: Text {
        color: (parent.hovered || parent.down || !parent.enabled) ? "white" : "#1bbc9d"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        color: control.enabled ? (control.down ? "#1bbc9d" : (control.hovered ? "#1bbc9d" : "white")) : "#2a2a2a"
        border.color: control.enabled ? "#1bbc9d" : "#2a2a2a"
        border.width: 2
    }
}

