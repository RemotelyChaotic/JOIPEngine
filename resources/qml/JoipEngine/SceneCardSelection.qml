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

    property var selectionColor: "white"
    property var filter: ".*"

    signal selectedScene(string scene)

    function onResize()
    {
        // nothing to do
    }

    function onUnLoad()
    {
        listModel.clear();
        gc();
    }

    function onAddScene(scene)
    {
        listModel.append({
            "scene": scene,
            "name": scene.name,
            "filter": scene.name + "-" + scene.sceneLayout + "-" + scene.titleCard,
            "type": "SceneCardDelegate.qml"
        });
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
