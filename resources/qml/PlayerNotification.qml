import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import JOIP.core 1.1
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
            "type": "NotificationDefaultDelegate.qml"
        });
    }

    function removeNotificaiton(sId)
    {
        if ("" !== sId)
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
    }

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
        property var backgroundColorWidget: "#ff111111"
        property var textColorWidget: "#ffffffff"
        property var iconAlignment: TextAlignment.AlignLeft
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

            sourceModel: ListModel {
                id: widgetModel
                dynamicRoles: true
            }

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

    Component.onCompleted: {
        ScriptRunner.registerNewComponent(userName, signalEmitter);
        registrator.componentLoaded();
    }
}
