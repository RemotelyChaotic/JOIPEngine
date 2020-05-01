import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.1
import JOIP.db 1.1
import SortFilterProxyModel 0.2

Rectangle {
    id: root
    color: "transparent"

    property Project currentlyAddedProject: null
    property var selectionColor: "white"
    property var filter: ".*"

    signal selectedProjectIndex(int index)

    function onResize()
    {
        // nothing to do
    }

    function onAddProject()
    {
        if (null !== currentlyAddedProject && undefined !== currentlyAddedProject)
        {
            listModel.append({
                "project": currentlyAddedProject,
                "name": currentlyAddedProject.name,
                "filter": currentlyAddedProject.name + "-" + currentlyAddedProject.describtion + "-" + currentlyAddedProject.kinks,
                "type": "ProjectCardDelegate.qml"
            });
        }
    }

    function onUnLoad()
    {
        listModel.clear();
    }

    ListView {
        id: listView
        anchors.centerIn: parent
        width: {
            var itemWidth = (height - 20) / 4 * 3;
            var neededWidth = (listModel.count * itemWidth + spacing * (listModel.count-1));
            return (neededWidth < parent.width - 20) ? neededWidth : parent.width - 20;
        }
        height: parent.height - 20
        clip: true

        layoutDirection: Qt.LeftToRight
        orientation: ListView.Horizontal
        spacing: 50

        signal clicked(int index)

        model: SortFilterProxyModel {
            id: listProxyModel

            sourceModel: listModel

            filters: RegExpFilter {
                id: sortFilter
                roleName: "filter"
                pattern: filter
                caseSensitivity: Qt.CaseInsensitive
            }

            sorters: StringSorter {
                id: sorter
                roleName: "name"
            }
        }

        delegate: Component {
            Loader {
                source: type
                asynchronous: true
            }
        }

        ScrollBar.horizontal: ScrollBar {}

        // Set the highlight delegate. Note we must also set highlightFollowsCurrentItem
        // to false so the highlight delegate can control how the highlight is moved.
        focus: true
    }


    ListModel {
        id: listModel
        dynamicRoles: false
    }
}
