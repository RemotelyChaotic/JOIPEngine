import QtQuick 2.14
import QtQuick.Templates 2.14

Button {
    contentItem: Text {
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        color: parent.enabled ? (parent.down ? "#32619f" : (parent.hovered ? "#488de6" : "#4281d2")) : "#344561"
        border.color: "#344561"
        border.width: 2
        radius: 5
    }
}

