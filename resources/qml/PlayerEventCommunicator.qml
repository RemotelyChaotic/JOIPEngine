import QtQuick 2.14
import JOIP.script 1.3

Item {
    id: reciever
    property string userName: "eventSender"

    function registerEventHandler(event, fnHandler)
    {
        if (null == handlers[event]) {
            handlers[event] = [fnHandler];
        }
        else {
            handlers[event].push(fnHandler);
        }
    }
    function sendEventResponse(event, response)
    {
        signalEmitter.sendReturnValue(response, event);
    }

    // no eval acessor object
    PlayerComponentRegistrator {
        id: registrator
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
        registrator.componentLoaded();
    }
}
