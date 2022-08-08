import QtQuick 2.14
import QtQuick.Controls 2.14
import JOIP.core 1.1

Rectangle {
    color: "transparent"

    property real maximumWidth: 1000000
    property string text: ""
    property var textColor: "#ffffff"

    width: Math.min(textContentItem.contentWidth, maximumWidth)
    height: textContentItem.contentHeight

    Text {
        id: textContentItem

        // yes, this causes a binding loop for property "width" too bad we ignore it
        // otherwise we can't get the width of the content
        anchors.fill: parent

        font.family: root.currentlyLoadedProject.font
        font.pointSize: 14
        font.hintingPreference: Font.PreferNoHinting

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideNone
        text: parent.text.replace("<html>","").replace("</html>","")
        wrapMode: Text.WordWrap
        color: parent.textColor
        textFormat: (parent.text.startsWith("<html>") && parent.text.endsWith("</html>")) ?
                        Text.RichText :
                        (QtApp.mightBeRichtext(parent.text) ?
                             Text.StyledText : Text.PlainText)
    }
}
