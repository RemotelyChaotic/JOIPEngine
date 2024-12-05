import QtQuick 2.14
import QtAV 1.7
import JOIP.core 1.5
import JOIP.db 1.1
import JOIP.script 1.1

Item {
    id: audioPlayer
    property int numberOfSoundEmitters : 0
    signal playbackFinishedCallback(string resource)

    function playMedia(sName, sId, iLoops, iStartAt, iEndAt) {
        if (null !== registrator.currentlyLoadedProject &&
            undefined !== registrator.currentlyLoadedProject)
        {
            var pResource = registrator.currentlyLoadedProject.resource(sName);
            if (null !== pResource && undefined !== pResource &&
                Resource.Sound === pResource.type && pResource.isLocal)
            {
                var soundPlayer = findFirstIdleSoundView();
                if (null !== soundPlayer && undefined !== soundPlayer)
                {
                    pResource.load(); // load bundles now
                    if (null !== soundPlayer.resource &&
                        sName === soundPlayer.resource.name)
                    {
                        soundPlayer.play();
                    }
                    else
                    {
                        soundPlayer.nameId = sId;
                        soundPlayer.loops = iLoops;
                        soundPlayer.startAt = iStartAt;
                        soundPlayer.endAt = iEndAt;
                        soundPlayer.resource = pResource;
                    }
                }
            }
            else
            {
                console.error(qsTr("Resources need to be local sound resources."));
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
        // no emitters found, cannibalize paused emitters
        for (var j = 0; soundRepeater.count > j; ++j)
        {
            var player2 = soundRepeater.itemAt(j);
            if (null !== player2 && undefined !== player2)
            {
                if (player2.playbackState === MediaPlayer.PausedState && player2.state === Resource.Loaded)
                {
                    return player2;
                }
            }
            else
            {
                console.error(qsTr("SoundResourceView should be valid here."));
            }
        }
        return null;
    }

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
                return true;
            }
        }
        if ("" === sResource || null === player)
        {
            for (var i = 0; soundRepeater.count > i; ++i)
            {
                var playerI = soundRepeater.itemAt(i);
                if (null !== playerI && undefined !== playerI &&
                    null !== playerI.resource)
                {
                    var fnToCallI = playerI[fn];
                    fnToCallI.apply(playerI, args);
                    return true;
                }
            }
        }
        return false;
    }

    PlayerComponentRegistrator {
        id: registrator

        onVolumeSet: {
            if (sResource === "")
            {
                for (var i = 0; soundRepeater.count > i; ++i)
                {
                    var player = soundRepeater.itemAt(i);
                    if (null !== player && undefined !== player)
                    {
                        player.setVolume(dValue);
                    }
                }
            }
            else
            {
                audioPlayer.tryToCall(sResource, "setVolume", dValue);
            }
        }

        onAudioTryToCallWith: {
            var bOk = tryToCall(sResource, fn, data);
            if (!bOk && null != fnNotFound)
            {
                fnNotFound();
            }
        }

        onPlayAudioRequested: {
            var pResource = registrator.currentlyLoadedProject.resource(sResource);
            if (null !== pResource && undefined !== pResource &&
                Resource.Sound === pResource.type)
            {
                SoundManager.registerId(sId || sResource, pResource, iLoops, iStartAt, iEndAt);
            }
            else
            {
                return;
            }

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
                    playMedia(sResource, sId, iLoops, iStartAt, iEndAt);
                }
            }
        }
    }

    // the sound
    Repeater {
        id: soundRepeater
        model: audioPlayer.numberOfSoundEmitters

        SoundResourceView {
            width: 0
            height: 0
            resource: null
            visible: true

            function checkIfFinished(res) {
                if (state === Resource.Null || state === Resource.Error)
                {
                    playbackFinishedCallback(res);
                }
            }

            onFinishedPlaying: {
                playbackFinishedCallback(resource.name);
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
    Connections {
        target: SoundManager
        onSignalPlay: {
            registrator.playAudioRequested(sResource, sId, iLoops, iStartAt, iEndAt);
        }
        onSignalPause: {
            tryToCall(sId, "pause");
        }
        onSignalStop: {
            tryToCall(sId, "stop");
        }
        onSignalSeek: {
            tryToCall(sId, "seek", dTime);
        }
        onSignalSetVolume: {
            tryToCall(sId, "setVolume", dVal);
        }
    }

    Component.onCompleted: {
        registrator.componentLoaded();
        root.registerUIComponent("Sound", SoundManager);
    }
}
