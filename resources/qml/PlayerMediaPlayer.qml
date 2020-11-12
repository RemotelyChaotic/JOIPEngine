import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtGui 5.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1
import QtAV 1.7

Rectangle {
    id: mediaPlayer
    color: "transparent"
    property string userName: "mediaPlayer"
    property bool mainMediaPlayer: false

    function showOrPlayMedia(sName) {
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
                    imgResource.resource = pResource;
                    break;

                case Resource.Movie:
                    resourceDisplay.source = pResource.source
                    imgResource.resource = null;
                    if (null !== movieResource.resource &&
                        sName === movieResource.resource.name)
                    {
                        movieResource.play();
                    }
                    else
                    {
                        movieResource.resource = pResource;
                    }
                    break;

                case Resource.Sound:
                    if (pResource.isLocal)
                    {
                        var soundPlayer = findFirstIdleSoundView();
                        if (null !== soundPlayer && undefined !== soundPlayer)
                        {
                            if (null !== soundPlayer.resource &&
                                sName === soundPlayer.resource.name)
                            {
                                soundPlayer.play();
                            }
                            else
                            {
                                soundPlayer.resource = pResource;
                            }
                        }
                    }
                    else
                    {
                        console.error(qsTr("Sound resources need to be local."));
                    }
                    break;

                default:
                    console.error(qsTr("Cannot display or play a resource of type other than 'Image' 'Movie' or 'Sound'"));
                    break;
                }
            }
        }
    }

    function findSoundView(sResource) {
        for (var i = 0; soundRepeater.count > i; ++i)
        {
            var player = soundRepeater.itemAt(i);
            if (null !== player && undefined !== player)
            {
                if (null !== player.resource && player.resource.name === sResource)
                {
                    return player;
                }
            }
            else
            {
                console.error(qsTr("SoundResourceView should be valid here."));
            }
        }
        return null;
    }

    function findFirstIdleSoundView() {
        for (var i = 0; soundRepeater.count > i; ++i)
        {
            var player = soundRepeater.itemAt(i);
            if (null !== player && undefined !== player)
            {
                if (player.state === Resource.Null ||
                    player.state === Resource.Error ||
                    player.playbackState === MediaPlayer.StoppedState && player.state === Resource.Loaded)
                {
                    return player;
                }
            }
            else
            {
                console.error(qsTr("SoundResourceView should be valid here."));
            }
        }
        return null;
    }

    MediaPlayerSignalEmitter {
        id: signalEmitter

        function tryToCall(sResource, fn) {
            var player = null;
            if ("" !== sResource)
            {
                player = findSoundView(sResource);
                if (null !== player)
                {
                    player[fn]();
                }
            }
            if ("" !== sResource || null === player)
            {
                for (var i = 0; soundRepeater.count > i; ++i)
                {
                    var playerI = soundRepeater.itemAt(i);
                    if (null !== playerI && undefined !== playerI &&
                        null !== playerI.resource)
                    {
                        playerI[fn]();
                    }
                }
            }
        }

        function tryToPlaySoundOrMovie(sResource) {
            if ("" !== sResource)
            {
                var player = findSoundView(sResource);
                if (null !== player && player.state === Resource.Loaded)
                {
                    if (null !== player.resource &&
                        sResource === player.resource.name)
                    {
                        player.play();
                    }
                    else
                    {
                        player.resource = pResource;
                    }
                }
                else
                {
                    showOrPlayMedia(sResource);
                }
            }
        }

        onPlayVideo: {
            movieResource.play();
        }
        onPlaySound: {
            if ("" !== sResource)
            {
                var pResource = registrator.currentlyLoadedProject.resource(sResource);
                if (null !== pResource && undefined !== pResource)
                {
                    if (Resource.Sound === pResource.type)
                    {
                        tryToPlaySoundOrMovie(sResource);
                    }
                }
            }
            else
            {
                signalEmitter.tryToCall(sResource, "play");
            }
        }
        onPauseVideo: {
            movieResource.pause();
        }
        onPauseSound: {
            signalEmitter.tryToCall(sResource, "pause");
        }
        onPlayMedia: {
            if ("" !== sResource)
            {
                tryToPlaySoundOrMovie(sResource);
            }
            else
            {
                if (null !== movieResource.resource)
                {
                    movieResource.play();
                }
                signalEmitter.tryToCall(sResource, "play");
            }
        }
        onShowMedia: {
            showOrPlayMedia(sResource);
        }
        onStopVideo: {
            movieResource.stop();
        }
        onStopSound: {
            signalEmitter.tryToCall(sResource, "stop");
        }
        onStartPlaybackWait: {
            var player = null;
            if ("" !== sResource)
            {
                player = findSoundView(sResource);
                if (null !== player)
                {
                    if (player.state === Resource.Null || player.state === Resource.Error)
                    {
                        player.playbackFinished(sResource);
                    }
                }
            }
            if ("" === sResource || null === player)
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
                var player = findSoundView(sResource);
                if (null !== player)
                {
                    if (player.state === Resource.Null || player.state === Resource.Error)
                    {
                        signalEmitter.soundFinished(sResource);
                    }
                }
                else
                {
                    signalEmitter.soundFinished(sResource);
                }
            }
            else
            {
                signalEmitter.soundFinished(sResource);
            }
        }
    }

    PlayerComponentRegistrator {
        id: registrator
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
            visible: !resourceDisplay.loading && !resourceDisplay.error
            hoverEnabled: true

            background: Rectangle {
                color: "transparent"
            }

            Image {
                id: iconWarningVersion
                anchors.fill: parent
                source: Settings.styleFolderQml() + "/InfoIcon.svg"
                opacity: iconSourceInfoRect.hovered ? 1.0 : 0.5
            }

            onClicked: {
                clipboard.text = resourceDisplay.source;
                iconSourceInfoRect.copied = true;
            }
        }
    }

    Repeater {
        id: soundRepeater
        model: registrator.currentlyLoadedProject.numberOfSoundEmitters

        SoundResourceView {
            width: 0
            height: 0
            resource: null
            visible: true

            onFinishedPlaying: {
                signalEmitter.playbackFinished(resource.name);
                signalEmitter.soundFinished(resource.name);
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
                for (var i = 0; soundRepeater.count > i; ++i)
                {
                    player = soundRepeater.itemAt(i);
                    if (null !== player && undefined !== player &&
                        player.state === Resource.Loaded &&
                        player.playbackState === MediaPlayer.PlayingState)
                    {
                        player.pause();
                    }
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
                for (var j = 0; soundRepeater.count > j; ++j)
                {
                    player = soundRepeater.itemAt(j);
                    if (null !== player && undefined !== player &&
                        player.state === Resource.Loaded &&
                        player.playbackState === MediaPlayer.PausedState)
                    {
                        player.play();
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        if (mainMediaPlayer)
        {
            registrator.registerMediaPlayer(mediaPlayer);
        }
        registrator.componentLoaded();
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
