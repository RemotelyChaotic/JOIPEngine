import QtQuick 2.14
import JOIP.core 1.1
import JOIP.db 1.1
import QtAV 1.7

Rectangle {
    id: mediaPlayer
    color: "transparent"

    property Resource resource: null
    property int state: Resource.Null
    property int playbackState: player.playbackState

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
            player.stoppedTargetState = true;
        }
    }

    onResourceChanged: {
        state = Resource.Null;
        player.stop();
        if (null !== resource && undefined !== resource)
        {
            state = Resource.Loading;
            if (resource.isLocal)
            {
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

        autoPlay: true
        videoCodecPriority: ["FFmpeg"]

        onPlaying: {
            player.stoppedTargetState = false;
            mediaPlayer.state = Resource.Loaded;
        }
        onStopped: {
            if (mediaPlayer.state === Resource.Loaded)
            {
                mediaPlayer.finishedPlaying();
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
        volume: stoppedTargetState ? 0 : Settings.volume

        Behavior on volume {
            animation: NumberAnimation {
                id:audiofadeout; duration: 1000; easing.type: Easing.InOutQuad
                onRunningChanged: {
                    if (!running && player.stoppedTargetState) {
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
                player.stop();
            }
            else if (status === MediaPlayer.InvalidMedia)
            {
                mediaPlayer.state = Resource.Error;
            }
        }
    }
}
