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

    property bool isHovered: false
    property bool isSelected: describtionDelegate.parent.ListView.isCurrentItem
    property Scene dto: scene

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
                root.selectedScene(dto.name);
                describtionDelegate.parent.ListView.view.currentIndex = index;
            }
            onEntered: isHovered = true
            onExited: isHovered = containsMouse
        }

        // title card
        TitleCard {
            id: card
            anchors.centerIn: parent
            width: parent.width
            height: parent.height

            resource: describtionDelegate.dto
            selectionColor: root.selectionColor
            isSelected: describtionDelegate.isSelected
            isHovered: describtionDelegate.isHovered
        }

        Rectangle {
            id: cardOverlayBackground
            height: 42
            width: parent.width
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#88000000"

            Text {
                id: nameText
                x: 20
                anchors.verticalCenter: parent.verticalCenter
                text: ""
                color: "white"
            }
        }
    }

    Component.onCompleted: {
        var pProject = dto.project();
        if (null != pProject) {
            var pResource = pProject.resource(dto.titleCard);
            if (null != pResource) {
                if (card.resource == null || card.resource.name !== pResource.name)
                {
                    card.resource = pResource;
                }
            }
            else {
                pResource = pProject.resource(pProject.titleCard);
                if (null != pResource) {
                    if (card.resource == null || card.resource.name !== pResource.name)
                    {
                        card.resource = pResource;
                    }
                }
            }
        }

        var tFormated = dto.name.replace(/\r/gi, "\n");
        nameText.text = tFormated;
        card.errorText = tFormated;
    }
}
