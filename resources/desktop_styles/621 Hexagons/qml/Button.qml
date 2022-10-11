import QtQuick 2.14
import QtQuick.Templates 2.14

Button {
    contentItem: Text {
        color: (parent.hovered || parent.down || !parent.enabled) ? "white" : "#21be2b"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        color: parent.enabled ? (parent.down ? "#152f56" : "#284a81") : "#031131"
        border.color: "#00759f"
        border.width: 2
        radius: 5
    }
}

