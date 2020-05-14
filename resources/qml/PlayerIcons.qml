import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: icon
    color: "green"
    property string userName: "icon"

    IconSignalEmitter {
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
