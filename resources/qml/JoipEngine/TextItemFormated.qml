import QtQuick 2.14
import QtQuick.Controls 2.14
import JOIP.core 1.5

Rectangle {
    id: textItemRoot
    color: "transparent"

    smooth: Settings.playerImageSmooth
    antialiasing: Settings.playerAntialiasing

    property real maximumWidth: 1000000
    property string text: ""
    property var textColor: "#ffffff"

    width: Math.min(textContentItem.contentWidth, maximumWidth)
    height: textContentItem.contentHeight

    Text {
        id: textContentItem

        property bool bIsHtml: textItemRoot.text.startsWith("<html>") && parent.text.endsWith("</html>")

        // yes, this causes a binding loop for property "width" too bad we ignore it
        // otherwise we can't get the width of the content
        anchors.centerIn: parent
        height: contentHeight
        width: Math.min(contentWidth, maximumWidth)

        font.family: null != root.currentlyLoadedProject ?
                         root.currentlyLoadedProject.font : "Arial"
        font.pointSize: 14
        font.hintingPreference: Font.PreferNoHinting

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideNone
        text: {
            var sText = textItemRoot.text.replace("<html>","").replace("</html>","")
                                         .replace("<body>", "").replace("</body>", "");
            if (bIsHtml) {
                if (contentWidth > textItemRoot.maximumWidth) {
                    sText = sText.replace("<nobr>","").replace("</nobr>","");
                }
            }
            return sText;
        }
        wrapMode: Text.WordWrap
        color: textItemRoot.textColor
        textFormat: (bIsHtml ?
                        Text.RichText :
                        (QtApp.mightBeRichtext(textItemRoot.text) ?
                             Text.StyledText : Text.PlainText))
    }
}
