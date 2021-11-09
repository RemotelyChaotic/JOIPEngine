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

    function showOrPlayMedia(sName, sId, iLoops, iStartAt) {
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
                                soundPlayer.loops = iLoops;
                                soundPlayer.startAt = iStartAt;
                                soundPlayer.resource = pResource;
                                soundPlayer.nameId = sId;
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
                if (null !== player.resource &&
                    (player.nameId !== "" && player.nameId === sResource ||
                     player.nameId === "" && player.resource.name === sResource))
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
            var args = Array.prototype.slice.call(arguments);
            args = args.slice(2);

            var player = null;
            if ("" !== sResource)
            {
                player = findSoundView(sResource);
                if (null !== player)
                {
                    var fnToCall = player[fn];
                    fnToCall.apply(player, args);
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
                        var fnToCallI = playerI[fn];
                        fnToCallI.apply(playerI, args);
                    }
                }
            }
        }

        function tryToPlaySoundOrMovie(sResource, sId, iLoops, iStartAt) {
            if ("" !== sResource || "" !== sId)
            {
                // first do id lookup
                var player = findSoundView(sId);
                if (null !== player && player.state === Resource.Loaded)
                {
                    if (null !== player.resource &&
                        sId === player.nameId)
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
                    // next do name lookup
                    var player2 = findSoundView(sResource);
                    if (null !== player2 && player2.state === Resource.Loaded)
                    {
                        if (null !== player2.resource &&
                            (sResource === player2.nameId || sResource === player2.resource.name))
                        {
                            player2.play();
                        }
                        else
                        {
                            player2.resource = pResource;
                        }
                    }
                    else
                    {
                        // if none found, start playing new media
                        showOrPlayMedia(sResource, sId, iLoops, iStartAt);
                    }
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
                tryToPlaySoundOrMovie(sResource, sId, iLoops, iStartAt);
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
                // could be both an id or a name
                tryToPlaySoundOrMovie(sResource, sResource, iLoops, iStartAt);
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
        onSeekAudio: {
            signalEmitter.tryToCall(sResource, "seek", iSeek);
        }
        onSeekMedia: {
            if (null !== movieResource.resource &&
                (sResource === movieResource.resource.name ||
                 sResource === ""))
            {
                movieResource.seek(iSeek);
            }
            signalEmitter.tryToCall(sResource, "seek", iSeek);
        }
        onSeekVideo: {
            movieResource.seek(iSeek);
        }
        onShowMedia: {
            showOrPlayMedia(sResource, "", 1, 0);
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

            property string nameId: ""

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
