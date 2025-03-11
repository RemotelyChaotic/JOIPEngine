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

    property bool isHtml: text.startsWith("<html>") && text.endsWith("</html>")
    property string textFormated: {
        var sText = text.replace("<html>","").replace("</html>","")
                        .replace("<body>", "").replace("</body>", "");
        if (isHtml) {
            if (metrics.boundingRect.width > maximumWidth) {
                sText = sText.replace("<nobr>","").replace("</nobr>","");
            }
        }
        return sText;
    }
    property int textFormat: (textItemRoot.isHtml ?
                                Text.RichText :
                                (QtApp.mightBeRichtext(textItemRoot.text) ?
                                     Text.StyledText : Text.PlainText))

    width: Math.min(metrics.boundingRect.width, maximumWidth)
    height: textContentItem.contentHeight

    TextMetrics {
        id: metrics
        text: QtApp.decodeHTML(textItemRoot.text)

        font.family: null != root.currentlyLoadedProject ?
                         root.currentlyLoadedProject.font : "Arial"
        font.pointSize: 14
        font.hintingPreference: Font.PreferNoHinting

        elide: Text.ElideNone
    }

    Text {
        id: textContentItem

        anchors.centerIn: parent
        height: contentHeight
        width: textItemRoot.width

        font.family: null != root.currentlyLoadedProject ?
                         root.currentlyLoadedProject.font : "Arial"
        font.pointSize: 14
        font.hintingPreference: Font.PreferNoHinting

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideNone
        text: textItemRoot.textFormated
        wrapMode: Text.WordWrap
        color: textItemRoot.textColor
        textFormat: textItemRoot.textFormat
    }
}
