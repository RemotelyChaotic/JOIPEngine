import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.5
import JOIP.db 1.5

Rectangle {
    id: achievements
    color: "transparent"

    property Project project: null
    property int itemHeight: 64
    property int popupTime: 5000

    property var popupSteps: [1, 2, 5]
    property var popupsToDo: []

    function updateAchievementView(sId, value, oldValue)
    {
        if (null != project)
        {
            var ach = project.achievement(sId);
            if (null == ach) { return; }

            // value check only pop up if the new value is greater than 1, 2, 5, 10, 20, 50 etc.
            // or if the value equals the max value
            var factor = 1;
            while (factor*10 < value) { factor *= 10; }
            for (var i = 0; popupSteps.length > i; ++i) {
                if (popupSteps[i]*factor > oldValue && popupSteps[i]*factor <= value)
                {
                    // we have a winner
                    popupsToDo.push({
                        "id": sId,
                        "value": value
                    });
                    popupOne();
                    break;
                }
            }
        }
    }

    function topPos() {
        var numItemsVisible = 0;
        if (ach1.visible) { numItemsVisible++; }
        if (ach2.visible) { numItemsVisible++; }
        if (ach3.visible) { numItemsVisible++; }
        return achievements.itemHeight * numItemsVisible;
    }

    function popupOne() {
        // check if we have an available achievement
        var item = null;
        var timer = null;
        if (!ach1.visible) { item = ach1; timer = ach1Timer; }
        if (!ach2.visible) { item = ach2; timer = ach2Timer; }
        if (!ach3.visible) { item = ach3; timer = ach3Timer; }

        if (null != item && popupsToDo.length > 0)
        {
            const firstElement = popupsToDo.shift();

            var res = null;
            var ach = project.achievement(firstElement.id);
            if (null != ach)
            {
                res = project.resource(ach.resource);
            }

            item.visible = true;
            item.save = ach;
            item.resource = res;
            item.y = topPos();
            item.value = firstElement.value;
            timer.start();
        }
    }

    AchievementPopupItem {
        id: ach1
        width: parent.width
        height: achievements.itemHeight
        x: 0
        y: parent.height

        visible: false

        Behavior on y {
            NumberAnimation { duration: 500; easing.type: Easing.OutQuart }
        }

        Timer {
            id: ach1Timer
            interval: achievements.popupTime; running: false; repeat: false
            onTriggered: {
                parent.visible = false;
                parent.save = null;
                parent.resource = null;
                parent.y = achievements.height;
                parent.value = 0;
            }
        }
    }
    AchievementPopupItem {
        id: ach2
        width: parent.width
        height: achievements.itemHeight
        x: 0
        y: parent.height

        visible: false

        Behavior on y {
            NumberAnimation { duration: 500; easing.type: Easing.OutQuart }
        }

        Timer {
            id: ach2Timer
            interval: achievements.popupTime; running: false; repeat: false
            onTriggered: {
                parent.visible = false;
                parent.save = null;
                parent.resource = null;
                parent.y = achievements.height;
                parent.value = 0;
            }
        }
    }
    AchievementPopupItem {
        id: ach3
        width: parent.width
        height: achievements.itemHeight
        x: 0
        y: parent.height

        visible: false

        Behavior on y {
            NumberAnimation { duration: 500; easing.type: Easing.OutQuart }
        }

        Timer {
            id: ach3Timer
            interval: achievements.popupTime; running: false; repeat: false
            onTriggered: {
                parent.visible = false;
                parent.save = null;
                parent.resource = null;
                parent.y = achievements.height;
                parent.value = 0;
            }
        }
    }
}
