import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.12
import JOIP.core 1.1
import JOIP.db 1.1

Rectangle {
    color: "transparent"

    property Resource resource: null
    property alias fontSize: text404.font.pixelSize

    property bool animated: false
    property bool startedLoad: false
    property bool loadingOk: true

    function startLoad(newResource)
    {
        loadingOk = true;
        startedLoad = true;
        if (null !== newResource && undefined !== newResource)
        {
            if (newResource.isLocal)
            {
                if (newResource.isAnimated)
                {
                    animatedImage.source = newResource.path;
                    animated = true
                }
                else
                {
                    image.source = "image://DataBaseImageProivider/" + newResource.project().id + "/" + newResource.name;
                    animated = false
                }
            }
            else
            {
                // TODO: Webview
            }
        }
        else
        {
            loadingOk = false;
        }
    }

    AnimatedImage {
        id: animation
        anchors.fill: parent
        visible: animatedImage.status === Image.Loading || image.status === Image.Loading || !startedLoad
        source: "qrc:/resources/gif/spinner_transparent.gif"
        fillMode: Image.Pad
    }

    Image {
        id: image
        anchors.centerIn: parent
        width: (status === Image.Ready && startedLoad && loadingOk) ? parent.width : 0
        height: (status === Image.Ready && startedLoad && loadingOk) ? parent.height : 0

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

        onStatusChanged: {
            if (startedLoad)
            {
                if (status === Image.Error || status === Image.Null)
                {
                    loadingOk = false;
                }
            }
        }

        fillMode: Image.PreserveAspectFit
        source: ""
    }

    AnimatedImage {
        id: animatedImage
        anchors.centerIn: parent
        width: (status === Image.Ready && startedLoad && loadingOk) ? parent.width : 0
        height: (status === Image.Ready && startedLoad && loadingOk) ? parent.height : 0

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

        onStatusChanged: {
            if (startedLoad)
            {
                if (status === Image.Error || status === Image.Null)
                {
                    loadingOk = false;
                }
            }
        }

        fillMode: Image.PreserveAspectFit
        source: ""
    }

    Rectangle {
        id: text404Rect
        anchors.centerIn: parent
        visible: !loadingOk
        width: visible ? parent.width : 0
        height: visible ? parent.height : 0

        color: "transparent"

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

        Text {
            id: text404
            text: "404"
            anchors.fill: parent
            color: "black"
            font.family: Settings.font
            font.pixelSize: 100
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            layer.enabled: true
            layer.effect: Glow {
                radius: 10
                samples: 17
                spread: 0.5
                color: "white"
                transparentBorder: false
            }
        }
    }
}
