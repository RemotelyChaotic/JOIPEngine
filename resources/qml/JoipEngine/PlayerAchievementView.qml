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
    property int orientation: Settings.dominantHand

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
            while (factor*10 <= value) { factor *= 10; }
            for (var i = 0; popupSteps.length > i; ++i) {
                if (popupSteps[i]*factor > oldValue && popupSteps[i]*factor <= value)
                {
                    // we have a winner
                    popupsToDo.push({
                        "id": sId,
                        "oldValue": oldValue,
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
        if (popupsToDo.length > 0)
        {
            // check if we have an available achievement
            var item = null;
            var anim = null;
            if (!ach1.visible) { item = ach1; anim = anim1; }
            if (!ach2.visible) { item = ach2; anim = anim2; }
            if (!ach3.visible) { item = ach3; anim = anim3; }

            if (null != item)
            {
                const firstElement = popupsToDo.shift();

                var res = null;
                var ach = project.achievement(firstElement.id);
                if (null != ach)
                {
                    res = project.resource(ach.resource);
                }

                item.y = topPos();
                item.visible = true;
                item.save = ach;
                item.resource = res;
                item.value = firstElement.oldValue
                anim.targetValue = firstElement.value;
                anim.start();
            }
        }
    }

    function moveUpDisplays() {
        var visibleByOrder = [];
        if (ach1.visible) visibleByOrder.push(ach1);
        if (ach2.visible) visibleByOrder.push(ach2);
        if (ach3.visible) visibleByOrder.push(ach3);
        visibleByOrder.sort((a, b) => { return a.y - b.y });
        for (var i = 0; visibleByOrder.length > i; ++i) {
            if (0 == i) { visibleByOrder[i].y = 0; }
            else {
                visibleByOrder[i].y = visibleByOrder[i-1].y + achievements.itemHeight;
            }
        }
    }

    AchievementPopupItem {
        id: ach1
        width: parent.width
        height: achievements.itemHeight
        x: 0
        y: parent.height

        visible: false

        orientation: achievements.orientation
        openState: 0.0

        Behavior on y {
            NumberAnimation { duration: 500; easing.type: Easing.OutBack}
        }

        SequentialAnimation {
            id: anim1
            running: false
            property real targetValue: 0
            PauseAnimation  { duration: 500; }
            NumberAnimation { target: ach1; property: "value"; to: anim1.targetValue; duration: 500; easing.type: Easing.OutQuart}
            NumberAnimation { target: ach1; property: "openState"; to: 1.0; duration: 500; easing.type: Easing.OutQuart}
            PauseAnimation  { duration: achievements.popupTime - 1000; }

            onFinished: {
                ach1.visible = false;
                ach1.openState = 0.0;
                ach1.save = null;
                ach1.resource = null;
                ach1.y = achievements.height;
                ach1.value = 0;

                achievements.moveUpDisplays();
                achievements.popupOne();
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

        orientation: achievements.orientation
        openState: 0.0

        Behavior on y {
            NumberAnimation { duration: 500; easing.type: Easing.OutBack}
        }

        SequentialAnimation {
            id: anim2
            running: false
            property real targetPos: achievements.height
            property real targetValue: 0
            PauseAnimation  { duration: 500; }
            NumberAnimation { target: ach2; property: "openState"; to: 1.0; duration: 500; easing.type: Easing.OutQuart}
            NumberAnimation { target: ach2; property: "value"; to: anim2.targetValue; duration: 500; easing.type: Easing.OutQuart}
            PauseAnimation  { duration: achievements.popupTime - 1000; }

            onFinished: {
                ach2.visible = false;
                ach2.openState = 0.0;
                ach2.save = null;
                ach2.resource = null;
                ach2.y = achievements.height;
                ach2.value = 0;

                achievements.moveUpDisplays();
                achievements.popupOne();
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

        orientation: achievements.orientation
        openState: 0.0

        Behavior on y {
            NumberAnimation { duration: 500; easing.type: Easing.OutBack}
        }

        SequentialAnimation {
            id: anim3
            running: false
            property real targetPos: achievements.height
            property real targetValue: 0
            PauseAnimation  { duration: 500; }
            NumberAnimation { target: ach3; property: "openState"; to: 1.0; duration: 500; easing.type: Easing.OutQuart}
            NumberAnimation { target: ach3; property: "value"; to: anim3.targetValue; duration: 500; easing.type: Easing.OutQuart}
            PauseAnimation  { duration: achievements.popupTime - 1000; }

            onFinished: {
                ach3.visible = false;
                ach3.openState = 0.0;
                ach3.save = null;
                ach3.resource = null;
                ach3.y = achievements.height;
                ach3.value = 0;

                achievements.moveUpDisplays();
                achievements.popupOne();
            }
        }
    }
}
