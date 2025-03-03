import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.5
import JOIP.db 1.5
import SortFilterProxyModel 0.2

Rectangle {
    id: achievementsView
    clip: true
    color: "transparent"

    property alias mouseArea: mouseAreaView
    property Project dto: null
    property var filter: ".*"
    property real itemSize: Math.max(Math.min(width / 3, 64),32)

    function formatTime(iTime)
    {
        var secs = iTime / 1000;
        var h = Math.floor(secs / 3600);
        var min = Math.floor((secs - h*3600) / 60);
        secs = Math.round(secs - h*3600 - min*60);
        return 0 < h ?
                    ("" + h + ":" + String(min).padStart(2, '0') + ":" + String(secs).padStart(2, '0')) :
                    ("" + String(min).padStart(2, '0') + ":" + String(secs).padStart(2, '0'));
    }

    function onAddAchievement(achievement)
    {
        if (null != dto && null != achievement)
        {
            var res = dto.resource(achievement.resource)
            var achValue = saveManager.load(achievement.name, "achievements");
            achValue = undefined !== achValue && null !== achValue ?
                        achValue : 0;
            switch(achievement.type)
            {
                case SaveData.Bool:
                    achValue = achValue ? 1 : 0; break;
                case SaveData.Int: // fallthrough
                default: break;
            }
            listModel.append({
                "name": achievement.name,
                "saveData": achievement,
                "resource": res,
                "saveValue": achValue,
                "filter": achievement.name + "-" + achievement.describtion,
                "type": "AchievementItemDelegate.qml"
            });
        }
    }

    SavegameManager {
        id: saveManager
        project: achievementsView.dto

        onProjectChanged: {
            listModel.clear();
            if (null != project) {

                var iPlay = saveManager.load("numPlayed", "stats");
                var iFin = saveManager.load("numFinished", "stats");
                var iTime = saveManager.load("playTime", "stats");

                iPlay = iPlay == null ? 0 : iPlay;
                iFin = iFin == null ? 0 : iFin;
                iTime = iTime == null ? 0 : iTime;

                textStats.text = "<h3>Stats:</h3>" +
                        "Play time: " + formatTime(iTime) + "<br>" +
                        "Number of times played: " + iPlay + "<br>" +
                        "Number of times finished: " + iFin;

                for (var i = 0; project.numAchievements() > i; ++i) {
                    var ach = project.achievement(i);
                    onAddAchievement(ach);
                }
            }
        }
    }

    MouseArea {
        id: mouseAreaView
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true
        propagateComposedEvents: true
        onWheel: {
            if (wheel.angleDelta.y < 0) {
                scrollV.increase();
            } else {
                scrollV.decrease();
            }
        }
        onPositionChanged: {
            let posInGridView = Qt.point(mouse.x, mouse.y);
            let posInContentItem = mapToItem(gridView.contentItem, posInGridView.x, posInGridView.y);
            let index = gridView.indexAt(posInContentItem.x, posInContentItem.y);
            gridView.currentIndex = index;
        }
        enabled: achievementsView.visible
    }

    Component {
        id: highlight
        Rectangle {
            color: "transparent"
            width: gridView.cellWidth
            height: gridView.cellHeight
            x: null != gridView.currentItem ? gridView.currentItem.x : 0
            y: null != gridView.currentItem ? gridView.currentItem.y : 0
            Rectangle {
                anchors.centerIn: parent
                width: parent.width-8
                height: parent.height-8
                color: root.selectionColor
                radius: width/2
            }
            Behavior on x { SpringAnimation { spring: 3; damping: 0.2 } }
            Behavior on y { SpringAnimation { spring: 3; damping: 0.2 } }

            layer.enabled: true
            layer.effect: Glow {
                radius: 8
                samples: 17
                spread: 0.5
                color: root.selectionColor
                transparentBorder: false
            }
            ToolTip {
                parent: parent
                visible: true
                text: {
                    var dtoAch = null != dto ?
                                 dto.achievement(gridView.currentIndex) : null;
                    return null != dtoAch ? ("<h3>" + qsTr(dtoAch.name) + "</h3><br>" + qsTr(dtoAch.describtion)) : "";
                }
            }
        }
    }

    Rectangle {
        id: stats
        color: "transparent"

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        height: parent.height < headerHeight ? parent.height : headerHeight

        property int headerHeight: 20*4+3*2

        TextEdit {
            id: textStats

            width: parent.width

            text: ""
            color: "white"
            wrapMode: TextEdit.Wrap
            readOnly: true
            textFormat: TextEdit.RichText
            selectByMouse: false
       }
    }

    GridView {
        id: gridView
        anchors.top: stats.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        height: parent.height - stats.height

        cacheBuffer: 0
        cellWidth: achievementsView.itemSize
        cellHeight: achievementsView.itemSize

        flow: GridView.FlowLeftToRight
        layoutDirection: Qt.LeftToRight
        verticalLayoutDirection: GridView.TopToBottom

        highlight: highlight
        highlightFollowsCurrentItem: false

        model: SortFilterProxyModel {
            id: listProxyModel

            sourceModel: listModel

            filters: RegExpFilter {
                id: sortFilter
                roleName: "filter"
                pattern: achievementsView.filter
                caseSensitivity: Qt.CaseInsensitive
            }

            sorters: StringSorter {
                id: sorter
                roleName: "name"
            }
        }

        ScrollBar.vertical: ScrollBar{
            id: scrollV
            orientation: Qt.Vertical
        }

        delegate: Component {
            Loader {
                source: type
                asynchronous: false
            }
        }
    }

    // needs to be declared outside because of https://bugreports.qt.io/browse/QTBUG-86428
    ListModel {
        id: listModel
        dynamicRoles: false
    }
}
