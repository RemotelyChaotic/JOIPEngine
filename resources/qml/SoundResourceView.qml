import QtQuick 2.14
import JOIP.core 1.2
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
    property string nameId: ""
    property SoundInstance soundInstance: null

    signal finishedPlaying();

    /*
    onPlaybackStateChanged: {
        var states =[];
        states[MediaPlayer.PlayingState] = "PlayingState";
        states[MediaPlayer.PausedState] = "PausedState";
        states[MediaPlayer.StoppedState] = "StoppedState";
        console.log("--------- playbackState: " + resource.name + ": " + states[playbackState]);
    }
    */

    function pause()
    {
        player.pauseRequested = true;
        player.stoppedTargetState = false;
        player.volume = Settings.volume;
        player.pause();
    }

    function play(iLoops, iStartAt)
    {
        player.pauseRequested = false;
        player.stoppedTargetState = false;
        player.volume = Settings.volume;
        player.play();
    }

    function stop()
    {
        player.pauseRequested = false;
        player.stoppedTargetState = true;
        player.volume = Settings.volume;
        player.volume = 0.0;
    }

    function seek(iSeekPos)
    {
        if (player.seekable)
        {
            player.fastSeek = true;
            player.seek(iSeekPos);
        }
    }

    onResourceChanged: {
        state = Resource.Null;
        player.stop();
        if (null !== resource && undefined !== resource)
        {
            soundInstance = SoundManager.get(nameId || resource.name);
            state = Resource.Loading;
            if (resource.isLocal)
            {
                player.pauseRequested = false;
                player.stoppedTargetState = false;
                player.volume = Settings.volume;
                player.source = resource.path;
            }
            else
            {
                state = Resource.Null;
            }
        }
        else
        {
            state = Resource.Null;
        }
    }

    AVPlayer {
        id: player
        objectName: "player"

        property int currentLoop: 0
        property bool pauseRequested: false

        loops: -1 === mediaPlayer.loops ? MediaPlayer.Infinite : mediaPlayer.loops
        startPosition: mediaPlayer.startAt
        autoPlay: true
        videoCodecPriority: ["FFmpeg"]

        onPlaying: {
            mediaPlayer.state = Resource.Loaded;
            if (pauseRequested) {
                pause();
            } else if (!stoppedTargetState) {
                soundInstance.dispatch(new Event("play"));
            }
        }
        onPaused: {
            pauseRequested = false;
            soundInstance.dispatch(new Event("pause"));
        }
        onStopped: {
            if (mediaPlayer.state === Resource.Loaded)
            {
                player.pauseRequested = false;
                player.stoppedTargetState = false;
                mediaPlayer.finishedPlaying();
                soundInstance.dispatch(new Event("end"));
            }
        }

        onError: {
            if (error != MediaPlayer.NoError) {
                console.error(errorString);
                mediaPlayer.state = Resource.Error;
            }
        }

        property bool stoppedTargetState: true
        muted: Settings.muted
        volume: stoppedTargetState ? 0.0 : Settings.volume

        Behavior on volume {
            animation: NumberAnimation {
                id:audiofadeout
                duration: 300
                easing.type: Easing.InOutQuad
                onRunningChanged: {
                    if (!running && player.stoppedTargetState) {
                        player.currentLoop = 0;
                        player.stop();
                    }
                }
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
    }
}
