import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: mediaPlayer
    color: "transparent"
    property string userName: "mediaPlayer"
    property bool mainMediaPlayer: false

    function showOrPlayMedia(sName)
    {
        if (null !== registrator.currentlyLoadedProject &&
            undefined !== registrator.currentlyLoadedProject)
        {
            var pResource = registrator.currentlyLoadedProject.resource(sName);
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

                case Resource.Sound:
                    if (pResource.isLocal)
                    {
                        soundResource.resource = pResource;
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

    MediaPlayerSignalEmitter {
        id: signalEmitter

        onPlayVideo: {
            movieResource.play();
        }
        onPlaySound: {
            soundResource.play();
        }
        onPauseVideo: {
            movieResource.pause();
        }
        onPauseSound: {
            soundResource.pause();
        }
        onPlayMedia: {
            if ("" !== sResource)
            {
                showOrPlayMedia(sResource);
            }
            else
            {
                if (movieResource.state === Resource.Loaded)
                {
                    movieResource.play();
                }
                else if (soundResource.state === Resource.Loaded)
                {
                    soundResource.play();
                }
            }
        }
        onShowMedia: {
            showOrPlayMedia(sResource);
        }
        onStopVideo: {
            movieResource.stop();
        }
        onStopSound: {
            soundResource.stop();
        }
        onStartPlaybackWait: {
            if (movieResource.state !== Resource.Loading && movieResource.state !== Resource.Loaded &&
                soundResource.state !== Resource.Loading && soundResource.state !== Resource.Loaded)
            {
                signalEmitter.playbackFinished();
            }
        }
        onStartVideoWait: {
            if (movieResource.state !== Resource.Loading && movieResource.state !== Resource.Loaded)
            {
                signalEmitter.videoFinished();
            }
        }
        onStartSoundWait: {
            if (soundResource.state !== Resource.Loading && soundResource.state !== Resource.Loaded)
            {
                signalEmitter.soundFinished();
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
                signalEmitter.playbackFinished();
                signalEmitter.videoFinished();
            }
        }

        ErrorResourceView {
            id: errorResource
            visible: resourceDisplay.error
            fontSize: 100
        }
    }

    SoundResourceView {
        id: soundResource
        width: 0
        height: 0
        resource: null
        visible: true

        onFinishedPlaying: {
            signalEmitter.playbackFinished();
            signalEmitter.soundFinished();
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
                if (soundResource.state === Resource.Loaded)
                {
                    soundResource.pause();
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
                if (soundResource.state === Resource.Loaded)
                {
                    soundResource.play();
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
}
