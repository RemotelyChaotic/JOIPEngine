import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: mediaPlayer
    color: "red"
    property string userName: "mediaPlayer"

    MediaPlayerSignalEmitter {
        id: signalEmitter
    }

    PlayerComponentRegistrator {
        id: registrator
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        registrator.componentLoaded();
    }
}
