import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1
import JOIP.db 1.1
import JOIP.script 1.1

Rectangle {
    id: icon
    color: "transparent"
    property string userName: "icon"

    function addIcon(sName)
    {
        if (null !== registrator.currentlyLoadedProject &&
            undefined !== registrator.currentlyLoadedProject)
        {
            var pResource = registrator.currentlyLoadedProject.resource(sName);
            if (null !== pResource && undefined !== pResource)
            {
                iconModel.append({
                    "name": pResource.name,
                    "resource": pResource
                });
            }
        }
    }

    function removeIcon(sName)
    {
        if (null !== registrator.currentlyLoadedProject &&
            undefined !== registrator.currentlyLoadedProject)
        {
            var pResource = registrator.currentlyLoadedProject.resource(sName);
            if (null !== pResource && undefined !== pResource)
            {
                var iIndex = iconModel.find(function(item){ return item.name === sName; });
                if (-1 !== iIndex)
                {
                    iconModel.remove(iIndex);
                }
            }
        }
    }

    function clearIcons(sName)
    {
        iconModel.clear();
    }


    IconSignalEmitter {
        id: signalEmitter

        onHideIcon: {
            if ("" === sResource || "~all" === sResource)
            {
                clearIcons();
            }
            else
            {
                removeIcon(sResource);
            }
        }

        onShowIcon: {
            if ("" !== sResource)
            {
                addIcon(sResource);
            }
        }
    }

    PlayerComponentRegistrator {
        id: registrator
    }

    ListModel {
        id: iconModel
        dynamicRoles: false

        function find(criteria) {
          for(var i = 0; i < count; ++i) if (criteria(get(i))) return i;
          return -1;
        }
    }

    // actual UI
    Rectangle {
        id: icons
        anchors.fill: parent
        color: "transparent"

        Flow {
            id: iconsFlow
            anchors.fill: parent
            spacing: 20
            flow: Flow.TopToBottom
            layoutDirection: Qt.LeftToRight

            Repeater {
                model: iconModel
                delegate: IconResourceDelegate {
                    pResource: model.resource
                }
            }
        }
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        registrator.componentLoaded();
    }
}
