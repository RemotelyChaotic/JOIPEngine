import QtQuick 2.14
import JOIP.core 1.5
import JOIP.script 1.3

Item {
    id: receiver
    property string userName: "eventSender"

    function registerEventHandler(event, fnHandler)
    {
        if (null == signalEmitter.handlers[event]) {
            signalEmitter.handlers[event] = [fnHandler];
        }
        else {
            signalEmitter.handlers[event].push(fnHandler);
        }
    }
    function sendEventResponse(event, response)
    {
        signalEmitter.sendReturnValue(response, event);
    }

    // no eval acessor object
    PlayerComponentRegistrator {
        id: registrator
        name: receiver.userName
    }

    EventSenderSignalEmitter {
        id: signalEmitter

        property var handlers: ({})

        onSendEvent: {
            if (null != handlers[event]) {
                for (let fn of handlers[event]) {
                    fn(dataJson);
                }
            }
        }
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
    }
}
