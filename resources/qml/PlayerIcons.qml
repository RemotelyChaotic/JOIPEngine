import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: icon
    color: "transparent"
    property string userName: "icon"

    function addIcon(sName)
    {
        if (null !== registrator.currentlyLoadedProject &&
            undefined !== registrator.currentlyLoadedProject)
        {
            var pResource = registrator.currentlyLoadedProject.resource(sName);
            if (null !== pResource && undefined !== pResource)
            {
                iconModel.append({
                    "name": pResource.name,
                    "resource": pResource
                });
            }
        }
    }

    function removeIcon(sName)
    {
        if (null !== registrator.currentlyLoadedProject &&
            undefined !== registrator.currentlyLoadedProject)
        {
            var pResource = registrator.currentlyLoadedProject.resource(sName);
            if (null !== pResource && undefined !== pResource)
            {
                var iIndex = iconModel.find(function(item){ return item.name === sName; });
                if (-1 !== iIndex)
                {
                    iconModel.remove(iIndex);
                }
            }
        }
    }

    function clearIcons(sName)
    {
        iconModel.clear();
    }


    IconSignalEmitter {
        id: signalEmitter

        onHideIcon: {
            if ("" === sResource || "~all" === sResource)
            {
                clearIcons();
            }
            else
            {
                removeIcon(sResource);
            }
        }

        onShowIcon: {
            if ("" !== sResource)
            {
                addIcon(sResource);
            }
        }
    }

    PlayerComponentRegistrator {
        id: registrator
    }

    ListModel {
        id: iconModel
        dynamicRoles: false

        function find(criteria) {
          for(var i = 0; i < count; ++i) if (criteria(get(i))) return i;
          return -1;
        }
    }

    // actual UI
    Rectangle {
        id: icons
        anchors.fill: parent
        color: "transparent"

        Flow {
            id: iconsFlow
            anchors.fill: parent
            spacing: 20
            flow: Flow.TopToBottom
            layoutDirection: Qt.LeftToRight

            Repeater {
                model: iconModel
                delegate: Item {
                    width: 64
                    height: 64

                    property Resource pResource: model.resource

                    Component.onCompleted: {
                        if (null !== pResource && undefined !== pResource)
                        {
                            switch (pResource.type)
                            {
                            case Resource.Image:
                                movieResource.resource = null;
                                imgResource.resource = pResource;
                                break;

                            case Resource.Movie:
                                imgResource.resource = null;
                                movieResource.resource = pResource;
                                break;

                            default:
                                console.error(qsTr("Cannot display or play a resource of type other than 'Image' or 'Movie'"));
                                break;
                            }
                            uiRootOfItem.width = width;
                            uiRootOfItem.height = height;
                            uiRootOfItem.rotation = 360;
                        }
                    }

                    // handle interrupt
                    Connections {
                        target: ScriptRunner
                        onRunningChanged: {
                            if (!bRunning)
                            {
                                if (imgResource.state === Resource.Loaded)
                                {
                                    imgResource.pause();
                                }
                                if (movieResource.state === Resource.Loaded)
                                {
                                    movieResource.pause();
                                }
                            }
                            else
                            {
                                if (imgResource.state === Resource.Loaded)
                                {
                                    imgResource.play();
                                }
                                if (movieResource.state === Resource.Loaded)
                                {
                                    movieResource.play();
                                }
                            }
                        }
                    }

                    Rectangle {
                        id: uiRootOfItem
                        anchors.centerIn: parent
                        width: 0
                        height: 0
                        color: "transparent"

                        rotation: 0
                        transformOrigin: Item.Center

                        Behavior on rotation {
                            SpringAnimation { spring: 2; damping: 0.2 }
                        }
                        Behavior on width {
                            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
                        }
                        Behavior on height {
                            NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
                        }

                        Image {
                            id: iconBG
                            anchors.fill: parent
                            source: Settings.styleFolderQml() + "/IconBg.svg";
                            sourceSize: Qt.size(parent.width, parent.height)
                            smooth: true
                            antialiasing: true
                            mipmap: true
                        }

                        Image {
                            id: iconBGOpacity
                            anchors.fill: parent
                            source: Settings.styleFolderQml() + "/IconBgMask.png";
                            sourceSize: Qt.size(parent.width, parent.height)
                            visible: false
                            antialiasing: true
                            mipmap: true
                        }

                        Rectangle {
                            id: resourceDisplay
                            anchors.fill: parent
                            color: "transparent"
                            visible: false

                            property bool loading: movieResource.state === Resource.Loading ||
                                                   imgResource.state === Resource.Loading
                            property bool error: imgResource.state === Resource.Null &&
                                                 movieResource.state === Resource.Null ||
                                                 imgResource.state === Resource.Error ||
                                                 movieResource.state === Resource.Error

                            AnimatedImage {
                                id: loadingAnimation
                                anchors.fill: parent
                                visible: resourceDisplay.loading
                                source: "qrc:/resources/gif/spinner_transparent.gif"
                                fillMode: Image.Stretch
                            }

                            ImageResourceView {
                                id: imgResource
                                anchors.centerIn: parent
                                width: parent.width
                                height: parent.height
                                resource: null
                                visible: true
                            }

                            MovieResourceView {
                                id: movieResource
                                anchors.fill: parent
                                resource: null
                                visible: true

                                onFinishedPlaying: {
                                    movieResource.play();
                                }
                            }
                        }

                        OpacityMask {
                            anchors.fill: resourceDisplay
                            source: resourceDisplay
                            maskSource: iconBGOpacity
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        registrator.componentLoaded();
    }
}
