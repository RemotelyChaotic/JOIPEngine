import QtQuick 2.14
import JOIP.core 1.2
import JOIP.db 1.1
import JOIP.script 1.2

Rectangle {
    id: sceneManager
    color: "transparent"
    property string userName: "sceneManager"

    SceneManagerSignalEmiter {
        id: signalEmitter

        onDisable: {
            sceneManagerPrivate.disable(sScene);
        }
        onEnable: {
            sceneManagerPrivate.enable(sScene);
        }
        onGotoScene: {
            sceneManagerPrivate.gotoScene(sScene);
        }
    }
    SceneManager {
        id: sceneManagerPrivate
    }
    property var pages: {
        "goto": function(sScene)
        {
            sceneManagerPrivate.gotoScene(sScene);
        },
        "getCurrentPageId": function()
        {
            return sceneManagerPrivate.getCurrentSceneId();
        },
        "disable": function(sScene)
        {
            sceneManagerPrivate.disable(sScene);
        },
        "enable": function(sScene)
        {
            sceneManagerPrivate.enable(sScene);
        },
        "isEnabled": function(sScene)
        {
            return sceneManagerPrivate.isEnabled(sScene);
        },
        "getCurrentSceneId": function()
        {
            return sceneManagerPrivate.getCurrentSceneId();
        },
        "gotoScene": function(sScene)
        {
            sceneManagerPrivate.gotoScene(sScene);
        },
        "addEventListener": function(sType, callback)
        {
            sceneManagerPrivate.addEventListener(sType, callback);
        },
        "removeEventListener": function(sType, callback)
        {
            sceneManagerPrivate.removeEventListener(sType, callback);
        },
        "dispatch": function(event)
        {
            sceneManagerPrivate.dispatch(event);
        }
    }

    PlayerComponentRegistrator {
        id: registrator
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        registrator.componentLoaded();

        Player.initObject(sceneManagerPrivate);

        // for whatever reason goto is a reserved word
        /*
        sceneManagerPrivate["goto"] = function(sScene)
        {
            gotoScene(sScene);
        };
        */

        root.registerUIComponent(sceneManager.userName, sceneManagerPrivate);
        // for compatibility with eos
        root.registerUIComponent("pages", pages);
    }
}
