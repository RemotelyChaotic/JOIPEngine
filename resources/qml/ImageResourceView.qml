import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.12
import JOIP.core 1.1
import JOIP.db 1.1

Rectangle {
    id: imageResource
    color: "transparent"

    property Resource resource: null
    property int state: Resource.Null
    property bool animated: false

    function play()
    {
        if (animated)
        {
            animatedImage.paused = false;
        }
    }

    function pause()
    {
        if (animated)
        {
            animatedImage.paused = true;
        }
    }

    onResourceChanged: {
        if (null !== resource && undefined !== resource)
        {
            state = Resource.Loading;
            if (resource.isAnimated)
            {
                animated = true;
                image.source = "";
                animatedImage.source = resource.path;
                play();
            }
            else
            {
                if (!resource.isLocal && Settings.offline)
                {
                    animated = false;
                    state = Resource.Null;
                }
                else
                {
                    animated = false;
                    animatedImage.source = "";
                    image.source = "image://DataBaseImageProivider/" + resource.project().id + "/" + resource.name;
                }
            }
        }
        else
        {
            state = Resource.Null;
        }
    }

    Image {
        id: image
        anchors.centerIn: parent
        width: (!imageResource.animated && imageResource.state === Resource.Loaded) ? parent.width : 0
        height: (!imageResource.animated && imageResource.state === Resource.Loaded) ? parent.height : 0

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
            if (!imageResource.animated)
            {
                if (status === Image.Error)
                {
                    imageResource.state = Resource.Error;
                }
                else if (status === Image.Ready)
                {
                    imageResource.state = Resource.Loaded;
                }
                else if (status === Image.Loading)
                {
                    imageResource.state = Resource.Loading;
                }
                else if (status === Image.Null)
                {
                    imageResource.state = Resource.Null;
                }
            }
        }

        antialiasing: true
        mipmap: true
        smooth: true
        fillMode: Image.PreserveAspectFit
        source: ""
    }

    AnimatedImage {
        id: animatedImage
        anchors.centerIn: parent
        width: (imageResource.animated && imageResource.state === Resource.Loaded) ? parent.width : 0
        height: (imageResource.animated && imageResource.state === Resource.Loaded) ? parent.height : 0

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
            if (imageResource.animated)
            {
                if (status === Image.Error)
                {
                    imageResource.state = Resource.Error;
                }
                else if (status === Image.Ready)
                {
                    imageResource.state = Resource.Loaded;
                }
                else if (status === Image.Loading)
                {
                    imageResource.state = Resource.Loading;
                }
                else if (status === Image.Null)
                {
                    imageResource.state = Resource.Null;
                }
            }
        }

        fillMode: Image.PreserveAspectFit
        source: ""
    }
}
