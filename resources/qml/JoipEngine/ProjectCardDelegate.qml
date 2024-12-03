import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Shapes 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.5
import JOIP.db 1.1

Rectangle {
    id: describtionDelegate
    width: parent.ListView.view.itemWidth
    height: parent.ListView.view.itemHeight
    clip: true
    color: "transparent"

    property int progress: -1
    property bool isHovered: false
    property bool isSelected: describtionDelegate.parent.ListView.isCurrentItem
    property bool openedDetails: false
    property Project dto: project

    Rectangle {
        id: cardRect
        anchors.centerIn: parent
        width: isHovered ? parent.width :  parent.width - 36
        height: isHovered ? parent.height : parent.height - 50

        Behavior on height {
            animation: ParallelAnimation {
                NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
            }
        }
        Behavior on width {
            animation: ParallelAnimation {
                NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
            }
        }

        radius: 10
        color: "transparent"

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                root.selectedProjectIndex(dto.id);
                describtionDelegate.parent.ListView.view.currentIndex = index;
            }
            onEntered: isHovered = true
            onExited: isHovered = false
        }

        // title card
        Rectangle {
            anchors.centerIn: parent
            width: parent.width
            height: parent.height

            color: "transparent"
            layer.enabled: describtionDelegate.isSelected
            layer.effect: Glow {
                radius: 8
                samples: 17
                spread: 0.5
                color: root.selectionColor
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
                resource: null

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
                                color: describtionDelegate.isSelected ? "#DDF0F0F0" : "#AAF0F0F0"
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
                fontSize: describtionDelegate.isHovered ? 100 : 90
            }
        }

        // ProgressBar
        Rectangle {
            id: progressRect
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            color: "transparent"
            visible: -1 !== describtionDelegate.progress

            Rectangle {
                id: progressDisplayRect
                anchors.left: parent.left
                height: parent.height
                width: parent.width * describtionDelegate.progress / 100.0
                opacity: 0.4

                Behavior on width {
                    animation: NumberAnimation {
                         duration: 500; easing.type: Easing.InOutQuad
                    }
                }

                gradient: Gradient {
                    id: gradientProgress
                    property double progressPosition: 0.0
                    SequentialAnimation on progressPosition {
                        loops: Animation.Infinite
                        PropertyAnimation {
                            from: 0.0
                            to: 1.0
                            duration: progressDisplayRect.width <= 0 ? 1000 : progressDisplayRect.width / 0.1
                        }
                        PropertyAnimation {
                            from: 1.0
                            to: 0.0
                            duration: progressDisplayRect.width <= 0 ? 1000 : progressDisplayRect.width / 0.1
                        }
                    }

                    orientation: Gradient.Horizontal

                    GradientStop {
                        position: Math.min(0.0, gradientProgress.progressPosition-0.3)
                        color: root.selectionColor
                    }
                    GradientStop {
                        position: gradientProgress.progressPosition
                        color: Qt.darker(root.selectionColor, 2.0)
                    }
                    GradientStop {
                        position: Math.max(1.0, gradientProgress.progressPosition+0.3)
                        color: root.selectionColor
                    }
                }
            }
        }

        // Overlay
        Rectangle {
            id: cardOverlay
            anchors.fill: parent
            color: "transparent"

            // Arrow
            Rectangle {
                height: 48
                width: 48
                x: cardOverlayBackground.x + cardOverlayBackground.width / 2 - 16
                y: cardOverlayBackground.y - (openedDetails ? -32 : 32)
                color: "transparent"

                Button {
                    id: buttonOpenOverlay
                    anchors.centerIn: parent
                    height: hovered ? 48 : 32
                    width: hovered ? 48 : 32
                    background: Image {
                        anchors.fill: parent
                        source: Settings.styleFolderQml() + (openedDetails ? "/ButtonArrowDown.svg" : "/ButtonArrowUp.svg")
                    }
                    onClicked: openedDetails = !openedDetails
                    onHoveredChanged: isHovered = true
                }
            }

            ColumnLayout {
                id: cardOverlayLayout
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    Layout.preferredHeight: parent.height - cardOverlayBackground.height
                    Layout.preferredWidth: cardOverlay.width
                    color: "transparent"
                }

                // background of overlay
                Rectangle {
                    id: cardOverlayBackground
                    Layout.preferredHeight: openedDetails ? parent.height : 42
                    Layout.preferredWidth: cardOverlay.width
                    color: "#88000000"

                    Behavior on Layout.preferredHeight {
                        animation: ParallelAnimation {
                            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
                        }
                    }


                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        // icons
                        Rectangle {
                            Layout.preferredHeight: 42
                            Layout.preferredWidth: parent.width - 20
                            Layout.alignment: Qt.AlignHCenter
                            color: "transparent"

                            Text {
                                id: versionText
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                text: ""
                                color: "white"
                            }

                            RowLayout {
                                id: icons
                                anchors.fill: parent

                                spacing: 0
                                layoutDirection: Qt.RightToLeft

                                Button {
                                    id: iconUsesCodecsRect
                                    Layout.preferredWidth: visible ? 32 : 0
                                    Layout.preferredHeight: 32
                                    Layout.alignment: Qt.AlignVCenter
                                    visible: false
                                    hoverEnabled: visible

                                    background: Rectangle {
                                        color: "transparent"
                                    }

                                    Image {
                                        id: iconUsesCodecs
                                        anchors.fill: parent
                                        source: Settings.styleFolderQml() + "/MediaIcon.svg"
                                    }

                                    onHoveredChanged: isHovered = true
                                }
                                Button {
                                    id: iconUsesWebRect
                                    Layout.preferredWidth: visible ? 32 : 0
                                    Layout.preferredHeight: 32
                                    Layout.alignment: Qt.AlignVCenter
                                    visible: false
                                    hoverEnabled: visible

                                    background: Rectangle {
                                        color: "transparent"
                                    }

                                    Image {
                                        id: iconUsesWeb
                                        anchors.fill: parent
                                        source: Settings.styleFolderQml() + "/WebIcon.svg"
                                    }

                                    onHoveredChanged: isHovered = true
                                }
                                Button {
                                    id: iconWarningVersionRect
                                    Layout.preferredWidth: visible ? 32 : 0
                                    Layout.preferredHeight: 32
                                    Layout.alignment: Qt.AlignVCenter
                                    visible: false
                                    hoverEnabled: visible

                                    background: Rectangle {
                                        color: "transparent"
                                    }

                                    Image {
                                        id: iconWarningVersion
                                        anchors.fill: parent
                                        source: Settings.styleFolderQml() + "/WarningIcon.svg"
                                    }

                                    onHoveredChanged: isHovered = true
                                }

                                Rectangle {
                                    id: spacerVert
                                    Layout.preferredWidth: parent.width
                                    Layout.preferredHeight: 32
                                    Layout.alignment: Qt.AlignVCenter
                                    color: "transparent"
                                }
                            }
                        }

                        Rectangle {
                            id: spacer
                            Layout.preferredHeight: openedDetails ? 48 : 0
                            Layout.preferredWidth: parent.width
                            color: "transparent"
                        }

                        // Detailed View
                        Flickable {
                             id: flick

                             Layout.preferredHeight: parent.height - icons.height - spacer.height
                             Layout.preferredWidth: parent.width - 20
                             Layout.alignment: Qt.AlignHCenter

                             contentWidth: describtion.paintedWidth
                             contentHeight: describtion.paintedHeight + kinkFlow.height
                             clip: true

                             visible: openedDetails

                             function ensureVisible(r)
                             {
                                 if (contentX >= r.x)
                                     contentX = r.x;
                                 else if (contentX+width <= r.x+r.width)
                                     contentX = r.x+r.width-width;
                                 if (contentY >= r.y)
                                     contentY = r.y;
                                 else if (contentY+height <= r.y+r.height)
                                     contentY = r.y+r.height-height;
                             }

                             // Describtion
                             TextEdit {
                                 id: describtion

                                 width: flick.width

                                 text: ""
                                 color: "white"
                                 wrapMode: TextEdit.Wrap
                                 readOnly: true
                                 textFormat: TextEdit.RichText
                                 selectByMouse: false

                                 onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)

                                 MouseArea {
                                    id: dummyMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        root.selectedProjectIndex(dto.id);
                                        describtionDelegate.parent.ListView.view.currentIndex = index;
                                    }
                                    onEntered: isHovered = true
                                    onExited: isHovered = true
                                 }
                            }

                             // Tags
                             Rectangle {
                                id: kinkFlow
                                color: "transparent"
                                y: describtion.height
                                width: flick.width
                                height: itemHeight * (rowCount+1) + spacing * rowCount

                                property int spacing: 5
                                property int itemHeight: textMetrics.boundingRect.height + 5
                                property int rowCount: repeater.rowCount

                                TextMetrics {
                                    id: textMetrics
                                    font.family: Settings.font
                                    font.pointSize: 9
                                    text: "testText here"
                                }

                                onWidthChanged: {
                                    repeater.currentRowWidth = 0;
                                    repeater.numAddedItems = 0;
                                    repeater.rowCount = 0;
                                    repeater.rowSizes = [];
                                    for (var i = 0; dto.numKinks() > i; ++i)
                                    {
                                        repeater.relayout(repeater.itemAt(i));
                                    }
                                }

                                Repeater {
                                    id: repeater
                                    model: dto.numKinks()

                                    property int currentRowWidth: 0
                                    property int numAddedItems: 0
                                    property int rowCount: 0
                                    property var rowSizes: []

                                    function relayout(item) {
                                        numAddedItems++;
                                        item.x = currentRowWidth;
                                        if (currentRowWidth + item.width + parent.spacing > parent.width)
                                        {
                                            rowSizes[rowCount] = currentRowWidth - parent.spacing;
                                            rowCount++;
                                            item.x = 0;
                                            currentRowWidth = item.width + parent.spacing;
                                        }
                                        else
                                        {
                                            currentRowWidth += item.width + parent.spacing;
                                        }
                                        item.y = rowCount * parent.itemHeight + rowCount * parent.spacing;
                                        item.rowIndex = rowCount;
                                        rowSizes[rowCount] = currentRowWidth - parent.spacing;

                                        // all items added
                                        if (numAddedItems === dto.numKinks())
                                        {
                                            for (var i = 0; i < dto.numKinks(); ++i)
                                            {
                                                var pItem = itemAt(i);
                                                var iRowWidth = rowSizes[pItem.rowIndex];
                                                pItem.x = pItem.x + (parent.width - iRowWidth) / 2;
                                            }
                                        }
                                    }

                                    // we need to calculate positions manually
                                    onItemAdded: {
                                        relayout(item);
                                    }

                                    Item {
                                        id: repeaterItem
                                        width: localTextMetrics.boundingRect.width + 20
                                        height: kinkFlow.itemHeight
                                        property int rowIndex: 0
                                        property Kink kink: dto.kink(dto.kinks()[index])
                                        property color kinkColor: kink.color()

                                        TextMetrics {
                                            id: localTextMetrics
                                            font.family: text.font.family
                                            font.pointSize: text.font.pointSize
                                            text: text.text
                                        }

                                        Rectangle {
                                            width: parent.width
                                            height: parent.height
                                            color: repeaterItem.kinkColor
                                            radius: 5
                                            border.color: "transparent"

                                            Text {
                                                id: text
                                                anchors.centerIn: parent
                                                font.family: Settings.font;
                                                font.pointSize: 9
                                                elide: Text.ElideNone
                                                text: repeaterItem.kink.name
                                                wrapMode: Text.WordWrap
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                                color: {
                                                    var dluminance =
                                                            (repeaterItem.kinkColor.r * 0.299 +
                                                             repeaterItem.kinkColor.g * 0.587 +
                                                             repeaterItem.kinkColor.b * 0.114);
                                                    return dluminance < 0.5 ? Qt.rgba(1,1,1,1) :  Qt.rgba(0,0,0,1);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    function updateDelegate(iProgress) {
        var pResource = dto.resource(dto.titleCard);
        if (null !== pResource && undefined !== pResource)
        {
            if (resource.resource == null || resource.resource.name !== pResource.name)
            {
                resource.resource = pResource;
            }
        }

        errorResource.text = dto.name.replace(/\r/gi, "\n");
        versionText.text = dto.versionText + " / " + dto.targetVersionText;
        describtion.text =
                "<h3>" + dto.name + "</h3>" +
                (dto.describtion.length > 0 ? dto.describtion : qsTr("<i>No describtion</i>")) +
                "<br><br>";
        if (dto.targetVersion > Settings.version)
        {
            iconWarningVersionRect.visible = true
        }
        if (dto.isUsingCodecs)
        {
            iconUsesCodecsRect.visible = true;
        }
        if (dto.isUsingWeb)
        {
            iconUsesWebRect.visible = true;
            desaturateEffect.desaturation = (Settings.offline && dto.isUsingWeb) ? 0.9 : 0.0;
        }

        describtionDelegate.progress = iProgress;
    }

    Component.onCompleted: {
        updateDelegate(-1);
    }

    Connections {
        target: root
        function onProjectUpdate(iProjId, iProgress) {
            if (iProjId === dto.id) {
                updateDelegate(iProgress);
            }
        }
    }

    ToolTip {
        parent: iconWarningVersionRect
        visible: iconWarningVersionRect.hovered
        text: qsTr("Target engine version and application version differ, project may not play properly.")
    }
    ToolTip {
        parent: iconUsesCodecsRect
        visible: iconUsesCodecsRect.hovered
        text: qsTr("Uses media with audio and might be loud.")
    }
    ToolTip {
        parent: iconUsesWebRect
        visible: iconUsesWebRect.hovered
        text: qsTr("Uses web resources and requires an internet connection.")
    }
}
