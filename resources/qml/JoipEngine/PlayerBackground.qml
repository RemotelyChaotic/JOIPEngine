import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: background
    color: "transparent"
    property string userName: "background"
    property bool startedBackgroundLoad: false
    property bool animated: false

    BorderImage {
        id: img
        source: ""
        anchors.fill: parent
        border { left: 0; top: 0; right: 0; bottom: 0 }
        horizontalTileMode: null == root.style ? BorderImage.Round : root.style.backgroundDisplay.horizontalTileMode
        verticalTileMode: null == root.style ? BorderImage.Round : root.style.backgroundDisplay.verticalTileMode

        visible: !background.animated

        onStatusChanged: {
            if (startedBackgroundLoad)
            {
                if ("" !== source)
                {
                    if (status === Image.Error || status === Image.Null)
                    {
                        console.error(qsTr("Resource %1 not found.").arg(source));
                        startedBackgroundLoad = false;
                    }
                    else if (status === Image.Ready)
                    {
                        startedBackgroundLoad = false;
                    }
                }
            }
        }
    }

    AnimatedImage {
        id: animatedImage
        source: ""

        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop

        visible: background.animated

        onStatusChanged: {
            if ("" !== source)
            {
                if (status === Image.Error || status === Image.Null)
                {
                    console.warn(qsTr("Resource %1 not found.").arg(source));
                    startedBackgroundLoad = false;
                }
                else if (status === Image.Ready)
                {
                    startedBackgroundLoad = false;
                }
            }
        }
    }

    ColorOverlay {
        id: colorOverlay
        anchors.fill: background.animated ? animatedImage : img
        source: background.animated ? animatedImage : img
        color: "#00000000"

        Behavior on color {
            ColorAnimation { duration: 1000 }
        }
    }

    Rectangle {
        id: gradient
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#96000000" }
            GradientStop { position: 0.4; color: "#32000000" }
            GradientStop { position: 0.6; color: "#32000000" }
            GradientStop { position: 1.0; color: "#96000000" }
        }
    }

    // accessor object for eval
    property var evalAccessor: ({
        setBackgroundColor: function(color)
        {
            signalEmitter.backgroundColorChanged(color);
        },
        setBackgroundTexture: function(sResource)
        {
            signalEmitter.backgroundTextureChanged(sResource);
        }
    });

    BackgroundSignalEmitter {
        id: signalEmitter

        onBackgroundColorChanged: {
            colorOverlay.color = color;
        }

        onBackgroundTextureChanged: {
            if (null !== registrator.currentlyLoadedProject &&
                undefined !== registrator.currentlyLoadedProject)
            {
                var pResource = registrator.currentlyLoadedProject.resource(sResource);
                if (null !== pResource && undefined !== pResource)
                {
                    if (!pResource.isAnimated)
                    {
                        background.animated = false;
                        startedBackgroundLoad = true;
                        img.source =  "image://DataBaseImageProivider/" +  + registrator.currentlyLoadedProject.id + "/" + sResource;
                    }
                    else
                    {
                        background.animated = true;
                        startedBackgroundLoad = true;
                        animatedImage.source = pResource.path;
                    }
                }
                else
                {
                    background.animated = false;
                    startedBackgroundLoad = true;
                    img.source = Settings.styleFolderQml() + "/Background.png";
                }
            }
            else
            {
                background.animated = false;
                startedBackgroundLoad = true;
                img.source = Settings.styleFolderQml() + "/Background.png";
            }
        }
    }

    PlayerComponentRegistrator {
        id: registrator
    }

    Component.onCompleted: {
        img.source = Settings.styleFolderQml() + "/Background.png";
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        registrator.componentLoaded();

        root.registerUIComponent(background.userName, evalAccessor);
    }
}
