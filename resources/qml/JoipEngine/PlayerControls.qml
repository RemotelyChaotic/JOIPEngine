import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtMultimedia 5.14
import JOIP.core 1.5

Rectangle {
    id: sceneControl
    color: "transparent"

    property int buttonHeight: 48
    property int buttonWidth: 64
    property int spacing: 10
    property Item soundEffects: root.soundEffects

    default property alias content: customContent.children

    signal setUiVisible(bool visible)

    RowLayout {
        anchors.fill: parent
        spacing: parent.spacing
        layoutDirection: DominantHand.Left === Settings.dominantHand ? Qt.LeftToRight :
                                                                       Qt.RightToLeft

        Button {
            id: exitButton
            Layout.preferredHeight: sceneControl.buttonHeight
            Layout.preferredWidth: sceneControl.buttonWidth
            Layout.alignment: Qt.AlignVCenter

            visible: !root.debug

            focusPolicy: Qt.NoFocus

            text: Settings.keyBinding("Exit")
            contentItem: Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
                rightPadding: 5
                bottomPadding: 5

                text: parent.text
                font.family: Settings.font
                font.pixelSize: 8
                color: "white"
            }

            function quit()
            {
                root.quit();
            }

            Image {
                anchors.centerIn: parent
                width: (parent.width < parent.height ? parent.width : parent.height) - 10
                height: (parent.width < parent.height ? parent.width : parent.height) - 10
                fillMode: Image.PreserveAspectFit
                source: Settings.styleFolderQml() + (parent.hovered ? "/ButtonExitHover.svg" : "/ButtonExit.svg")
            }

            onHoveredChanged: {
                if (hovered)
                {
                    soundEffects.hoverSound.play();
                }
            }
            onClicked: {
                soundEffects.clickSound.play();
                quit();
            }

            Shortcut {
                sequence: Settings.keyBinding("Exit")
                context: Qt.ApplicationShortcut
                onActivated: {
                    pauseButton.quit();
                }
            }
        }

        Button {
            id: pauseButton
            Layout.preferredHeight: sceneControl.buttonHeight
            Layout.preferredWidth: sceneControl.buttonWidth
            Layout.alignment: Qt.AlignVCenter

            focusPolicy: Qt.NoFocus

            text: Settings.keyBinding("Pause")
            contentItem: Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
                rightPadding: 5
                bottomPadding: 5

                text: parent.text
                font.family: Settings.font
                font.pixelSize: 8
                color: "white"
            }

            property bool paused: false
            function pausePlayScene()
            {
                if (pauseButton.paused)
                {
                    ScriptRunner.resumeExecution();
                }
                else
                {
                    ScriptRunner.pauseExecution();
                }
            }

            Image {
                anchors.centerIn: parent
                width: (parent.width < parent.height ? parent.width : parent.height) - 10
                height: (parent.width < parent.height ? parent.width : parent.height) - 10
                fillMode: Image.PreserveAspectFit
                source: Settings.styleFolderQml() +
                        (parent.paused ? (parent.hovered ? "/ButtonPlayHover.svg" : "/ButtonPlay.svg") :
                                         (parent.hovered ? "/ButtonPauseHover.svg" : "/ButtonPause.svg"))
            }

            onHoveredChanged: {
                if (hovered)
                {
                    soundEffects.hoverSound.play();
                }
            }
            onClicked: {
                soundEffects.clickSound.play();
                pausePlayScene();
            }

            Shortcut {
                sequence: Settings.keyBinding("Pause")
                context: Qt.ApplicationShortcut
                onActivated: {
                    pauseButton.pausePlayScene();
                }
            }
        }

        Button {
            id: hideButton
            Layout.preferredHeight: sceneControl.buttonHeight
            Layout.preferredWidth: sceneControl.buttonWidth
            Layout.alignment: Qt.AlignVCenter

            focusPolicy: Qt.NoFocus

            text: Settings.keyBinding("ToggleUI")
            contentItem: Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
                rightPadding: 5
                bottomPadding: 5

                text: parent.text
                font.family: Settings.font
                font.pixelSize: 8
                color: "white"
            }

            property bool visibleUi: true

            Image {
                anchors.centerIn: parent
                width: (parent.width < parent.height ? parent.width : parent.height) - 10
                height: (parent.width < parent.height ? parent.width : parent.height) - 10
                fillMode: Image.PreserveAspectFit
                source: Settings.styleFolderQml() +
                        (parent.visibleUi ? (parent.hovered ? "/ButtonVisibleHover.svg" : "/ButtonVisible.svg") :
                                            (parent.hovered ? "/ButtonInvisibleHover.svg" : "/ButtonInvisible.svg"))
            }

            onHoveredChanged: {
                if (hovered)
                {
                    soundEffects.hoverSound.play();
                }
            }
            onClicked: {
                soundEffects.clickSound.play();
                hideButton.visibleUi = !hideButton.visibleUi;
                sceneControl.setUiVisible(hideButton.visibleUi);
            }

            Shortcut {
                sequence: Settings.keyBinding("ToggleUI")
                context: Qt.ApplicationShortcut
                onActivated: {
                    hideButton.visibleUi = !hideButton.visibleUi;
                    sceneControl.setUiVisible(hideButton.visibleUi);
                }
            }
        }

        Button {
            id: devicesButton
            Layout.preferredHeight: sceneControl.buttonHeight
            Layout.preferredWidth: sceneControl.buttonWidth
            Layout.alignment: Qt.AlignVCenter

            focusPolicy: Qt.NoFocus

            text: Settings.keyBinding("Devices")
            contentItem: Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
                rightPadding: 5
                bottomPadding: 5

                text: parent.text
                font.family: Settings.font
                font.pixelSize: 8
                color: "white"
            }

            Image {
                anchors.centerIn: parent
                width: (parent.width < parent.height ? parent.width : parent.height) - 10
                height: (parent.width < parent.height ? parent.width : parent.height) - 10
                fillMode: Image.PreserveAspectFit
                source: Settings.styleFolderQml() + (parent.hovered ? "/ButtonPlugHover.png" : "/ButtonPlug.png")
            }

            visible: TeaseDeviceController.numberRegisteredConnectors > 0
            onHoveredChanged: {
                if (hovered)
                {
                    soundEffects.hoverSound.play();
                }
            }
            onClicked: {
                soundEffects.clickSound.play();
                var globalCoordinares = devicesButton.mapToItem(root, 0, devicesButton.height);
                TeaseDeviceController.openSelectionScreen(globalCoordinares.x, globalCoordinares.y);
            }

            Shortcut {
                sequence: Settings.keyBinding("Devices")
                context: Qt.ApplicationShortcut
                onActivated: {
                    var globalCoordinares = devicesButton.mapToItem(root, 0, devicesButton.height);
                    TeaseDeviceController.openSelectionScreen(globalCoordinares.x, globalCoordinares.y);
                }
            }
        }

        RowLayout {
            id: customContent
            height: parent.height
            spacing: parent.spacing
            layoutDirection: DominantHand.Left === Settings.dominantHand ? Qt.LeftToRight :
                                                                           Qt.RightToLeft
        }

        Rectangle {
            id: spacerHoriz
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            Layout.alignment: Qt.AlignVCenter
            color: "transparent"
        }
    }

    Connections {
        target: ScriptRunner
        onRunningChanged: {
            pauseButton.paused = !bRunning;
        }
    }
}
