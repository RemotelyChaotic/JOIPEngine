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

    property int iconWidth: 64
    property int iconHeight: 64

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

    function clearIcons()
    {
        iconModel.clear();
    }

    // accessor object for eval
    property var evalAccessor: ({
        addIcon: function(sName)
        {
            icon.clearTextBox(sName);
        },
        removeIcon: function(sName)
        {
            icon.clearTextBox(sName);
        },
        clearIcons: function()
        {
            icon.clearIcons();
        },
        hideIcon: function(sName)
        {
            signalEmitter.hideIcon(sName);
        },
        showIcon: function(sName)
        {
            signalEmitter.showIcon(sName);
        }
    });

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
            flow: Flow.LeftToRight
            layoutDirection: Qt.LeftToRight

            Repeater {
                model: iconModel
                delegate: IconResourceDelegate {
                    pResource: model.resource
                    width: icon.iconWidth
                    height: icon.iconHeight
                }
            }
        }
    }

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        registrator.componentLoaded();

        root.registerUIComponent(icon.userName, evalAccessor);
    }
}
