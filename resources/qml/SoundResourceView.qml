import QtQuick 2.14
import JOIP.core 1.1
import JOIP.db 1.1
import QtAV 1.7

Rectangle {
    id: mediaPlayer
    color: "transparent"

    property Resource resource: null
    property int state: Resource.Null

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
            player.stop();
        }
    }

    onResourceChanged: {
        mediaPlayer.stop();
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

        onStopped: {
            mediaPlayer.finishedPlaying();
        }

        onError: {
            if (error != MediaPlayer.NoError) {
                console.error(errorString);
                mediaPlayer.state = Resource.Error;
            }
        }
        muted: Settings.muted
        volume: Settings.volume

        onStatusChanged: {
            if (status === MediaPlayer.Loading)
            {
                mediaPlayer.state = Resource.Loading;
            }
            else if (status === MediaPlayer.Buffering)
            {
                mediaPlayer.state = Resource.Loaded;
            }
            else if (status === MediaPlayer.Buffered)
            {
                mediaPlayer.state = Resource.Loaded;
            }
            else if (status === MediaPlayer.EndOfMedia)
            {
                mediaPlayer.state = Resource.Loaded;
                player.stop();
            }
            else if (status === MediaPlayer.InvalidMedia)
            {
                mediaPlayer.state = Resource.Error;
            }
        }
    }
}