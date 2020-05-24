import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.1
import JOIP.db 1.1

Rectangle {
    id: describtionDelegate
    width: height / 4 * 3
    height: parent.ListView.view.height - 20
    clip: true
    color: "transparent"

    property bool isHovered: false
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

        Rectangle {
            anchors.centerIn: parent
            width: parent.width
            height: parent.height

            color: "transparent"
            layer.enabled: describtionDelegate.parent.ListView.isCurrentItem
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
            }

            ErrorResourceView {
                id: errorResource
                visible: resource.state === Resource.Null || resource.state === Resource.Error
                fontSize: describtionDelegate.isHovered ? 100 : 90
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

                        // Describtion
                        Flickable {
                             id: flick

                             Layout.preferredHeight: parent.height - icons.height - spacer.height
                             Layout.preferredWidth: parent.width - 20
                             Layout.alignment: Qt.AlignHCenter

                             contentWidth: describtion.paintedWidth
                             contentHeight: describtion.paintedHeight
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
                        }
                    }

                }
            }
        }
    }


    Component.onCompleted: {
        var pResource = dto.resource(dto.titleCard);
        if (null !== pResource && undefined !== pResource)
        {
            resource.resource = pResource;
        }

        var sFetishesElem = qsTr("Fetishes:") + "<ul>%1</ul>";
        var sFetishes = "";
        for (var i = 0; i < dto.kinks.length; i++)
        {
            sFetishes += "<li>" + dto.kinks[i] + "</li>";
        }

        versionText.text = dto.versionText + " / " + dto.targetVersionText;
        describtion.text = (dto.describtion.length > 0 ? dto.describtion : qsTr("<i>No describtion</i>")) +
                "<br><br>" +
                (dto.kinks.length > 0 ? sFetishesElem.arg(sFetishes) : "");
        if (dto.targetVersion !== Settings.version)
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
        text: qsTr("Uses web resources and requires an internet connection.")
    }
    ToolTip {
        parent: iconUsesWebRect
        visible: iconUsesWebRect.hovered
        text: qsTr("Uses web resources and requires an internet connection.")
    }
}
