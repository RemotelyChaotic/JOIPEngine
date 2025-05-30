import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.12
import JOIP.core 1.5
import JOIP.db 1.1
import SortFilterProxyModel 0.2

Rectangle {
    id: root
    color: "transparent"
    property var style: null;

    property Project currentlyAddedProject: null
    property int currentlyRemovedProject: -1
    property var selectionColor: "white"
    property var filter: ".*"

    signal selectedProjectIndex(int index)
    signal projectUpdate(int iProjId, int iProgress)

    function onResize()
    {
        // nothing to do
    }

    function findProject(id)
    {
        if (-1 !== id)
        {
            for (var i = 0; i < listModel.count; ++i)
            {
                if (listModel.get(i).project.id === id)
                {
                    return i;
                }
            }
        }
        return -1;
    }

    function onAddProject()
    {
        if (null !== currentlyAddedProject && undefined !== currentlyAddedProject)
        {
            listModel.append({
                "project": currentlyAddedProject,
                "name": currentlyAddedProject.name,
                "filter": currentlyAddedProject.name + "-" + currentlyAddedProject.describtion + "-" + currentlyAddedProject.kinks(),
                "type": "ProjectCardDelegate.qml"
            });
        }
    }

    function onRemoveProject()
    {
        var iIndex = findProject(currentlyRemovedProject);
        if (-1 !== iIndex)
        {
            listModel.remove(iIndex);
        }
    }

    function onUpdateProject(iProjId, iProgress)
    {
        root.projectUpdate(iProjId, iProgress);
    }

    function onUnLoad()
    {
        listModel.clear();
        currentlyAddedProject = null;
        currentlyRemovedProject = -1;
        gc();
    }

    function showWarning(sWarnString)
    {
        warningPopup.text = sWarnString;
        warningPopup.open();
    }

    MouseArea {
        anchors.fill: parent
        preventStealing: true
        propagateComposedEvents: true
        onWheel: {
            if (wheel.angleDelta.y < 0) {
                listView.isLandscape ? scrollH.increase() : scrollV.increase();
            } else {
                listView.isLandscape ? scrollH.decrease() : scrollV.decrease();
            }
        }
    }

    ListView {
        id: listView
        anchors.centerIn: parent

        property bool isLandscape: parent.width > parent.height
        property real itemWidth: isLandscape ? (parent.height - 40) / 4 * 3 : parent.width - 40
        property real itemHeight: isLandscape ? parent.height - 40 : (parent.width - 40) / 4 * 3
        property real neededWidth: (listModel.count * itemWidth + spacing * (listModel.count-1))
        property real neededHeight: (listModel.count * itemHeight + spacing * (listModel.count-1))

        width: isLandscape ? ((neededWidth < parent.width - 20) ? neededWidth : parent.width - 20) :
                             (parent.width - 20)
        height: isLandscape ? (parent.height - 20) :
                              ((neededHeight < parent.height - 20) ? neededHeight : parent.height - 20)
        clip: true

        cacheBuffer: 0
        layoutDirection: Qt.LeftToRight
        orientation: isLandscape ? ListView.Horizontal : ListView.Vertical
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

        ScrollBar.horizontal: ScrollBar{
            id: scrollH

            visible: parent.isLandscape
            orientation: Qt.Horizontal
        }
        ScrollBar.vertical: ScrollBar{
            id: scrollV

            visible: !parent.isLandscape
            orientation: Qt.Vertical
        }

        // Set the highlight delegate. Note we must also set highlightFollowsCurrentItem
        // to false so the highlight delegate can control how the highlight is moved.
        focus: true
    }

    ToolTip {
        id: warningPopup
        anchors.centerIn: parent
        modal: false
        focus: true
        text: ""
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    }

    // needs to be declared outside because of https://bugreports.qt.io/browse/QTBUG-86428
    ListModel {
        id: listModel
        dynamicRoles: false
    }

    Component.onCompleted: {
        var styleComponent = Qt.createComponent(Settings.styleFolderQml() + "/Style.qml");
        if (styleComponent.status === Component.Ready)
        {
            root.style = styleComponent.createObject(root);
            if (root.style === null) {
                console.error(qsTr("Error creating Style object"));
            }
        }
    }
}
