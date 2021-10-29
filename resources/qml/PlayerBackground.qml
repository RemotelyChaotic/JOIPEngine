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

    BorderImage {
        id: img
        source: ""
        anchors.fill: parent
        border { left: 0; top: 0; right: 0; bottom: 0 }
        horizontalTileMode: null == root.style ? BorderImage.Round : root.style.backgroundDisplay.horizontalTileMode
        verticalTileMode: null == root.style ? BorderImage.Round : root.style.backgroundDisplay.verticalTileMode

        onStatusChanged: {
            if (startedBackgroundLoad)
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

    ColorOverlay {
        id: colorOverlay
        anchors.fill: img
        source: img
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

    BackgroundSignalEmitter {
        id: signalEmitter

        onBackgroundColorChanged: {
            colorOverlay.color = color;
        }

        onBackgroundTextureChanged: {
            if (sResource !== "")
            {
                startedBackgroundLoad = true;
                img.source =  "image://DataBaseImageProivider/" +  + registrator.currentlyLoadedProject.id + "/" + sResource;
            }
            else
            {
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
    }
}
