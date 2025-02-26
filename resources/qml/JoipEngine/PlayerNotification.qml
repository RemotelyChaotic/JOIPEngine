import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.5
import JOIP.db 1.1
import JOIP.script 1.1
import SortFilterProxyModel 0.2

Rectangle {
    id: notification
    color: "transparent"
    property string userName: "notification"

    function showNotification(sId, sTitle, sButtonText, iTimeS, sOnButton, sOnTimeout)
    {
        widgetModel.append({
            "sId": sId,
            "title": sTitle,
            "buttonText": sButtonText,
            "timeMs": iTimeS*1000,
            "sOnButton": sOnButton,
            "sOnTimeout": sOnTimeout,
            "backgroundColor": widgetDisplay.backgroundColor,
            "textColor": widgetDisplay.textColor,
            "backgroundColorWidget": widgetDisplay.backgroundColorWidget,
            "textColorWidget": widgetDisplay.textColorWidget,
            "iconAlignment": widgetDisplay.iconAlignment,
            "portrait": widgetDisplay.portrait,
            "shortcut": widgetDisplay.shortcut,
            "type": "NotificationDefaultDelegate.qml"
        });
    }

    function findNotification(sId)
    {
        for (var i = 0; i < widgetModel.count; ++i)
        {
            if (widgetModel.get(i).sId === sId)
            {
                return widgetModel.get(i)
            }
        }
    }

    function removeNotificaiton(sId)
    {
        for (var i = 0; i < widgetModel.count; ++i)
        {
            if (widgetModel.get(i).sId === sId)
            {
                widgetModel.remove(i);
                return;
            }
        }
    }

    // accessor object for eval
    property var evalAccessor: ({
        showNotification: function(sId, sTitle, sButtonText, iTimeS, sOnButton, sOnTimeout)
        {
            notification.showNotification(sId, sTitle, sButtonText, iTimeS, sOnButton, sOnTimeout);
        },
        removeNotificaiton: function(sId)
        {
            notification.removeNotificaiton(sId);
        },
        clearNotifications: function()
        {
            signalEmitter.clearNotifications();
        },
        hideNotification: function(sId)
        {
            signalEmitter.hideNotification(sId);
        },
        setIconAlignment: function(alignment)
        {
            signalEmitter.iconAlignmentChanged(alignment);
        },
        setPortrait: function(sResource)
        {
            signalEmitter.portraitChanged(sResource);
        },
        setShortcut: function(sShortcut)
        {
            signalEmitter.shortcutChanged(sShortcut);
        },
        setTextBackgroundColor: function(color)
        {
            signalEmitter.textBackgroundColorChanged(color);
        },
        setTextColor: function(color)
        {
            signalEmitter.textColorChanged(color);
        },
        setWidgetBackgroundColor: function(color)
        {
            signalEmitter.widgetBackgroundColorChanged(color);
        },
        setWidgetColor: function(color)
        {
            signalEmitter.widgetColorChanged(color);
        }
    });

    NotificationSignalEmiter {
        id: signalEmitter

        onClearNotifications: {
            widgetModel.clear();
        }
        onHideNotification: {
            notification.removeNotificaiton(sId);
        }
        onIconAlignmentChanged: {
            widgetDisplay.iconAlignment = alignment;
        }
        onPortraitChanged: {
            if (null !== registrator.currentlyLoadedProject &&
                undefined !== registrator.currentlyLoadedProject)
            {
                if (sResource === "")
                {
                    widgetDisplay.portrait = null;
                }
                else
                {
                    var pResource = registrator.currentlyLoadedProject.resource(sResource);
                    if (null !== pResource && undefined !== pResource)
                    {
                        widgetDisplay.portrait = pResource;
                    }
                    else
                    {
                        widgetDisplay.portrait = null;
                    }
                }
            }
        }
        onShowNotification: {
            notification.showNotification(sId, sTitle, sButtonText, iTimeS, sOnButton, sOnTimeout);
        }
        onShortcutChanged: {
            widgetDisplay.shortcut = sShortcut;
        }
        onTextBackgroundColorChanged: {
            widgetDisplay.backgroundColor = color;
        }
        onTextColorChanged: {
            widgetDisplay.textColor = color;
        }
        onWidgetBackgroundColorChanged: {
            widgetDisplay.backgroundColorWidget = color;
        }
        onWidgetColorChanged: {
            widgetDisplay.textColorWidget = color;
        }
    }

    PlayerComponentRegistrator {
        id: registrator
    }

    // Actual UI
    ListView {
        id: widgetDisplay
        anchors.fill: parent

        cacheBuffer: 0
        orientation: ListView.Vertical
        property var backgroundColor: "#ff000000"
        property var textColor: "#ffffffff"
        property var backgroundColorWidget: "#ff333333"
        property var textColorWidget: "#ffffffff"
        property var iconAlignment: TextAlignment.AlignLeft
        property string shortcut: ""
        property Resource portrait: null

        function buttonPressed(sId, sOnButton)
        {
            signalEmitter.showNotificationClick(sId, sOnButton);
            notification.removeNotificaiton(sId);
        }

        function timeout(sId, sOnTimeout)
        {
            signalEmitter.showNotificationTimeout(sId, sOnTimeout);
            notification.removeNotificaiton(sId);
        }

        // we want the notifications sorted by id
        model: SortFilterProxyModel {
            id: widgetProxyModel

            sourceModel: widgetModel
            filters: RegExpFilter {
                id: sortFilter
                roleName: "sId"
                pattern: ".*"
                caseSensitivity: Qt.CaseInsensitive
            }

            sorters: StringSorter {
                id: sorter
                roleName: "sId"
            }
        }

        delegate: Component {
            Loader {
                source: type
                asynchronous: false
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: textScrollBar
            visible: true
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
    }

    // needs to be declared outside because of https://bugreports.qt.io/browse/QTBUG-86428
    ListModel {
        id: widgetModel
        dynamicRoles: true
    }

    Connections {
        target: NotificationManager
        onSignalRemove: {
            notification.removeNotificaiton(sId);
        }
        onSignalSetTitle: {
            var item = notification.findNotification(sId);
            if (null !== item && undefined !== item)
            {
                item.title = sTitle;
                return;
            }
        }
    }


    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        registrator.componentLoaded();

        root.registerUIComponent(notification.userName, evalAccessor);
        root.registerUIComponent("Notification", NotificationManager);
    }
}
