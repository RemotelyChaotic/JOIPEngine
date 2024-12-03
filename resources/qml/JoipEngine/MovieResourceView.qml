import QtQuick 2.14
import JOIP.core 1.5
import JOIP.db 1.1
import QtAV 1.7

Rectangle {
    id: mediaPlayer
    color: "transparent"

    property Resource resource: null
    property int state: Resource.Null
    property int playbackState: player.playbackState
    property int loops: 1
    property int startAt: 0
    property int endAt: -1
    property double volume: 1.0

    signal finishedPlaying();

    function pause()
    {
        if (player.playbackState === MediaPlayer.PlayingState)
        {
            player.pause();
        }
    }

    function play()
    {
        if (player.playbackState !== MediaPlayer.PlayingState)
        {
            player.play();
        }
    }

    function stop()
    {
        if (player.playbackState === MediaPlayer.PlayingState || player.playbackState === MediaPlayer.PausedState)
        {
            player.currentLoop = 0;
            player.stop();
        }
    }

    function seek(iSeekPos)
    {
        if ((player.playbackState === MediaPlayer.PlayingState || player.playbackState === MediaPlayer.PausedState) && player.seekable)
        {
            player.fastSeek = true;
            player.seek(iSeekPos);
        }
    }

    function setVolume(dVolume)
    {
        mediaPlayer.volume = dVolume;
    }

    onResourceChanged: {
        state = Resource.Null;
        mediaPlayer.stop();
        player.currentLoop = 0;
        if (null !== resource && undefined !== resource)
        {
            if (!resource.isLocal && Settings.offline)
            {
                state = Resource.Null;
            }
            else
            {
                state = Resource.Loading;
                mediaPlayer.volume = 1.0;
                player.source = resource.path;
            }
        }
        else
        {
            state = Resource.Null;
        }
    }

    VideoOutput2 {
        id: videoOut
        anchors.centerIn: parent
        width: mediaPlayer.state === Resource.Loaded ? parent.width : 0
        height: mediaPlayer.state === Resource.Loaded ? parent.height : 0
        backgroundColor: "transparent"

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

        opengl: true
        fillMode: VideoOutput.PreserveAspectFit
        source: player
        orientation: 0
        property real zoom: 1
        //filters: [negate, hflip]
        /*
        SubtitleItem {
            id: subtitleItem
            fillMode: videoOut.fillMode
            rotation: -videoOut.orientation
            source: subtitle
            anchors.fill: parent
        }
        Text {
            id: subtitleLabel
            rotation: -videoOut.orientation
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignBottom
            font: PlayerConfig.subtitleFont
            style: PlayerConfig.subtitleOutline ? Text.Outline : Text.Normal
            styleColor: PlayerConfig.subtitleOutlineColor
            color: PlayerConfig.subtitleColor
            anchors.fill: parent
            anchors.bottomMargin: PlayerConfig.subtitleBottomMargin
        }*/
    }

    AVPlayer {
        id: player
        objectName: "player"

        property int currentLoop: 0

        loops: -1 === mediaPlayer.loops ? MediaPlayer.Infinite : mediaPlayer.loops
        startPosition: mediaPlayer.startAt
        stopPosition: -1 == mediaPlayer.endAt ? MediaPlayer.PositionMax : mediaPlayer.endAt
        //autoLoad: true
        autoPlay: true
        videoCodecPriority: ["FFmpeg"]
        //onPositionChanged: control.setPlayingProgress(position/duration)
        //videoCapture {
        //    autoSave: true
        //    onSaved: {
        //        msg.info("capture saved at: " + path)
        //    }
        //}
        onSourceChanged: {
            videoOut.zoom = 1
            videoOut.regionOfInterest = Qt.rect(0, 0, 0, 0)
            //msg.info("url: " + source)
        }

        //onDurationChanged: control.duration = duration
        //onPlaying: {
        //    control.mediaSource = player.source
        //    control.setPlayingState()
        //    if (!pageLoader.item)
        //        return
        //    if (pageLoader.item.information) {
        //        pageLoader.item.information = {
        //            source: player.source,
        //            hasAudio: player.hasAudio,
        //            hasVideo: player.hasVideo,
        //            metaData: player.metaData
        //        }
        //    }
        //}
        //onSeekFinished: {
        //    console.log("seek finished " + Utils.msec2string(position))
        //}

        //onInternalAudioTracksChanged: {
        //    if (typeof(pageLoader.item.internalAudioTracks) != "undefined")
        //        pageLoader.item.internalAudioTracks = player.internalAudioTracks
        //}
        //onExternalAudioTracksChanged: {
        //    if (typeof(pageLoader.item.externalAudioTracks) != "undefined")
        //        pageLoader.item.externalAudioTracks = player.externalAudioTracks
        //}
        //onInternalSubtitleTracksChanged: {
        //    if (typeof(pageLoader.item.internalSubtitleTracks) != "undefined")
        //        pageLoader.item.internalSubtitleTracks = player.internalSubtitleTracks
        //}

        onPlaying: {
            mediaPlayer.state = Resource.Loaded;
        }
        onStopped: {
            if (mediaPlayer.state === Resource.Loaded)
            {
                mediaPlayer.finishedPlaying();
            }
        }
        //onPaused: control.setPauseState()
        onError: {
            if (error != MediaPlayer.NoError) {
                console.error(errorString);
                mediaPlayer.state = Resource.Error;
            }
        }
        muted: Settings.muted
        volume: Settings.muted ? 0.0 : Settings.volume * mediaPlayer.volume

        Behavior on volume {
            animation: NumberAnimation {
                id: audiofade
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }

        onStatusChanged: {
            if (status === MediaPlayer.Loading)
            {
                mediaPlayer.state = Resource.Loading;
            }
            else if (status === MediaPlayer.EndOfMedia)
            {
                currentLoop++;
                if (currentLoop >= loops)
                {
                    player.stop();
                }
            }
            else if (status === MediaPlayer.InvalidMedia)
            {
                mediaPlayer.state = Resource.Error;
            }
        }
        //onBufferProgressChanged: {
        //    msg.info("Buffering " + Math.floor(bufferProgress*100) + "%...")
        //}
        //onSeekFinished: msg.info("Seek finished: " + Utils.msec2string(position))
    }
}
