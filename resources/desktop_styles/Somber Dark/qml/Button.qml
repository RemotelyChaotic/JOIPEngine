import QtQuick 2.14
import QtQuick.Templates 2.14

Button {
    id: control
    padding: 20

    contentItem: Text {
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Image {
        anchors.fill: parent
        source: control.enabled ? (control.down ? "IconBgHover.svg" : (control.hovered ? "IconBgHover.svg" : "IconBg.svg")) : "IconBgHover.svg"
        fillMode: Image.PreserveAspectFit
        smooth: true
    }
}

