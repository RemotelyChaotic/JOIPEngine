import QtQuick 2.14
import QtQuick.Templates 2.14 as T

T.Popup {
    id: control

    x: parent ? (parent.width - implicitWidth) / 2 : 0
    y: -implicitHeight - 3

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    margins: 6
    padding: 20

    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutside | T.Popup.CloseOnReleaseOutside

    background: BorderImage {
        anchors.fill: parent
        border { left: 25; top: 25; right: 25; bottom: 25 }
        horizontalTileMode: BorderImage.Round
        verticalTileMode: BorderImage.Round
        source: "WindowBGSmall.png"
        smooth: true
    }
}
