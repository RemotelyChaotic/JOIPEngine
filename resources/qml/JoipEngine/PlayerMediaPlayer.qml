import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtGui 5.14
import QtAV 1.7
import JOIP.core 1.5
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: mediaPlayer
    color: "transparent"
    property string userName: "mediaPlayer"
    property bool mainMediaPlayer: false

    function showOrPlayMedia(sName, sId, iLoops, iStartAt, iEndAt) {
        if (null !== registrator.currentlyLoadedProject &&
            undefined !== registrator.currentlyLoadedProject)
        {
            var pResource = registrator.currentlyLoadedProject.resource(sName);
            if (null !== pResource && undefined !== pResource)
            {
                switch (pResource.type)
                {
                case Resource.Image:
                    resourceDisplay.source = pResource.source
                    movieResource.resource = null;
                    if (null === imgResource.resource ||
                        sName !== imgResource.resource.name)
                    {
                        imgResource.resource = pResource;
                    }
                    break;

                case Resource.Movie:
                    pResource.load(); // load bundles now
                    resourceDisplay.source = pResource.source
                    imgResource.resource = null;
                    if (null !== movieResource.resource &&
                        sName === movieResource.resource.name)
                    {
                        movieResource.play();
                    }
                    else
                    {
                        movieResource.loops = iLoops;
                        movieResource.startAt = iStartAt;
                        movieResource.endAt = iEndAt;
                        movieResource.resource = pResource;
                    }
                    break;

                default:
                    console.error(qsTr("Cannot display or play a resource of type other than 'Image' 'Movie' or 'Sound'"));
                    break;
                }
            }
        }
    }

    // accessor object for eval
    property var evalAccessor: ({
        showOrPlayMedia: function(sName, sId, iLoops, iStartAt, iEndAt)
        {
            mediaPlayer.showOrPlayMedia(sName, sId, iLoops, iStartAt, iEndAt);
        },
        tryToPlaySoundOrMovie: function(sResource, sId, iLoops, iStartAt, iEndAt)
        {
            signalEmitter.tryToPlaySoundOrMovie(sResource, sId, iLoops, iStartAt, iEndAt);
        },
        playVideo: function()
        {
            signalEmitter.playVideo();
        },
        playSound: function(sResource, sId, iLoops, iStartAt, iEndAt)
        {
            signalEmitter.playSound(sResource, sId, iLoops, iStartAt, iEndAt);
        },
        pauseVideo: function()
        {
            signalEmitter.pauseVideo();
        },
        pauseSound: function(sResource)
        {
            signalEmitter.pauseSound(sResource);
        },
        playMedia: function(sResource, sResource, iLoops, iStartAt, iEndAt)
        {
            signalEmitter.playMedia(sResource, sResource, iLoops, iStartAt, iEndAt);
        },
        seekAudio: function(sResource, iSeek)
        {
            signalEmitter.seekAudio(sResource, iSeek);
        },
        seekMedia: function(sResource, iSeek)
        {
            signalEmitter.seekMedia(sResource, iSeek);
        },
        seekVideo: function(iSeek)
        {
            signalEmitter.seekVideo(iSeek);
        },
        setVolume: function(sResource, dValue)
        {
            signalEmitter.setVolume(sResource, dValue);
        },
        showMedia: function(sResource)
        {
            signalEmitter.showMedia(sResource, "", 1, 0, -1);
        },
        stopVideo: function()
        {
            signalEmitter.stopVideo();
        },
        stopSound: function(sResource)
        {
            signalEmitter.stopSound(sResource);
        }
    });

    MediaPlayerSignalEmitter {
        id: signalEmitter

        function tryToPlaySoundOrMovie(sResource, sId, iLoops, iStartAt, iEndAt) {
            if ("" !== sResource || "" !== sId)
            {
                var pResource = registrator.currentlyLoadedProject.resource(sResource);
                if (null !== pResource && undefined !== pResource &&
                    Resource.Sound !== pResource.type)
                {
                    showOrPlayMedia(sResource, sId, iLoops, iStartAt, iEndAt);
                }
                else if (Resource.Sound === pResource.type)
                {
                    registrator.playAudio(sResource, sId, iLoops, iStartAt, iEndAt);
                }
            }
        }

        onPlayVideo: {
            movieResource.play();
        }
        onPlaySound: {
            if ("" !== sResource || "" !== sId)
            {
                // both could be an id, so just make a normal lookup
                tryToPlaySoundOrMovie(sResource, sId, iLoops, iStartAt, iEndAt);
            }
            else
            {
                registrator.tryToCallAudio(sResource, "play");
            }
        }
        onPauseVideo: {
            movieResource.pause();
        }
        onPauseSound: {
            registrator.tryToCallAudio(sResource, "pause");
        }
        onPlayMedia: {
            if ("" !== sResource)
            {
                // could be both an id or a name
                tryToPlaySoundOrMovie(sResource, sResource, iLoops, iStartAt, iEndAt);
            }
            else
            {
                if (null !== movieResource.resource)
                {
                    movieResource.play();
                }
                registrator.tryToCallAudio(sResource, "play");
            }
        }
        onSeekAudio: {
            registrator.tryToCallAudio(sResource, "seek", iSeek);
        }
        onSeekMedia: {
            if (null !== movieResource.resource &&
                (sResource === movieResource.resource.name ||
                 sResource === ""))
            {
                movieResource.seek(iSeek);
            }
            registrator.tryToCallAudio(sResource, "seek", iSeek);
        }
        onSeekVideo: {
            movieResource.seek(iSeek);
        }
        onSetVolume: {
            if (null !== movieResource.resource &&
                (sResource === movieResource.resource.name ||
                 sResource === ""))
            {
                movieResource.setVolume(dValue);
            }

            registrator.setVolume(sResource, dValue);
        }
        onShowMedia: {
            showOrPlayMedia(sResource, "", 1, 0, -1);
        }
        onStopVideo: {
            movieResource.stop();
        }
        onStopSound: {
            registrator.tryToCallAudio(sResource, "stop");
        }
        onStartPlaybackWait: {
            var player = null;
            if ("" !== sResource)
            {
                registrator.tryToCallAudio(sResource, "checkIfFinished", sResource,
                                           function(state, fnFinished) {
                                                return function() {
                                                    if (state === Resource.Null || state === Resource.Error)
                                                    {
                                                        fnFinished("");
                                                    }
                                                }
                                           }(movieResource.state, signalEmitter.playbackFinished));
            }
            if ("" === sResource)
            {
                if (movieResource.state === Resource.Null || movieResource.state === Resource.Error)
                {
                    signalEmitter.playbackFinished("");
                }
            }
        }
        onStartVideoWait: {
            if (movieResource.state === Resource.Null || movieResource.state === Resource.Error)
            {
                signalEmitter.videoFinished();
            }
        }
        onStartSoundWait: {
            if ("" !== sResource)
            {
                registrator.tryToCallAudio(sResource, "checkIfFinished", sResource,
                                           function(fnFinished, _sResource) {
                                                return function() {
                                                    fnFinished(_sResource);
                                                }
                                           }(signalEmitter.playbackFinished, sResource));
            }
            else
            {
                signalEmitter.soundFinished(sResource);
            }
        }
    }

    PlayerComponentRegistrator {
        id: registrator
        name: mediaPlayer.userName

        onSoundFinished: {
            signalEmitter.playbackFinished(sResource);
            signalEmitter.soundFinished(sResource);
        }
    }

    // actual UI
    Rectangle {
        id: resourceDisplay
        anchors.fill: parent
        color: "transparent"

        property bool loading: movieResource.state === Resource.Loading ||
                               imgResource.state === Resource.Loading
        property bool error: imgResource.state === Resource.Null &&
                             movieResource.state === Resource.Null &&
                             imgResource.state === Resource.Error ||
                             movieResource.state === Resource.Error
        property string source: ""

        AnimatedImage {
            id: loadingAnimation
            anchors.fill: parent
            visible: resourceDisplay.loading
            source: "qrc:/resources/gif/spinner_transparent.gif"
            fillMode: Image.Pad
        }

        ImageResourceView {
            id: imgResource
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            resource: null
            visible: imgResource.state === Resource.Loaded
        }

        MovieResourceView {
            id: movieResource
            anchors.fill: parent
            resource: null
            visible: movieResource.state === Resource.Loaded

            onFinishedPlaying: {
                signalEmitter.playbackFinished("");
                signalEmitter.videoFinished();
            }
        }

        ErrorResourceView {
            id: errorResource
            visible: resourceDisplay.error
            fontSize: 100
        }

        Button {
            id: iconSourceInfoRect
            anchors.right: parent.right
            anchors.top: parent.top
            width: 32
            height: 32
            visible: !resourceDisplay.loading && !resourceDisplay.error && "" !== resourceDisplay.source
            hoverEnabled: true

            background: Rectangle {
                color: "transparent"
            }

            Image {
                id: iconWarningVersion
                anchors.fill: parent
                source: Settings.styleFolderQml() + "/InfoIcon.svg"
                opacity: iconSourceInfoRect.hovered ? 1.0 : 0.5

                smooth: Settings.playerImageSmooth
                antialiasing: Settings.playerAntialiasing
                mipmap: Settings.playerImageMipMap
            }

            onClicked: {
                clipboard.text = resourceDisplay.source;
                iconSourceInfoRect.copied = true;
            }
        }
    }

    // handle interrupt
    Connections {
        target: ScriptRunner
        onRunningChanged: {
            var player = null;
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
    Connections {
        target: root
        onStartLoadingSkript: {
            if (null != registrator.currentlyLoadedProject)
            {
                showOrPlayMedia(registrator.currentlyLoadedProject.titleCard);
            }
        }
    }

    Component.onCompleted: {
        registrator.registerNewScriptComponent(signalEmitter);
        registrator.registerNewUiComponent(evalAccessor);
    }

    // Misc components
    ToolTip {
        parent: iconSourceInfoRect
        visible: "" !== resourceDisplay.source && iconSourceInfoRect.hovered
        text: qsTr("Source: ") + resourceDisplay.source + (copied ? "\nCopied to clipboard" : "")
        property bool copied: false
        onCopiedChanged: {
            copyTimer.start();
        }
    }

    Timer {
        id: copyTimer
        interval: 2000; running: false; repeat: false
        onTriggered: {
            iconSourceInfoRect.copied = false;
        }
    }

    Clipboard {
        id: clipboard
    }
}
