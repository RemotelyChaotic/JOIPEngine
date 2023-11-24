import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtMultimedia 5.14
import JOIP.core 1.1

Rectangle {
    id: sceneControl
    color: "transparent"

    property int buttonHeight: 48
    property int buttonWidth: 64
    property int spacing: 10
    property Item soundEffects: null

    default property alias content: customContent.children

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

            text: Settings.keyBinding("Exit")
            contentItem: Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
                rightPadding: 5
                bottomPadding: 5

                text: parent.text
                font.family: Settings.font
                font.pixelSize: 10
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

            text: Settings.keyBinding("Pause")
            contentItem: Text {
                anchors.fill: parent
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
                rightPadding: 5
                bottomPadding: 5

                text: parent.text
                font.family: Settings.font
                font.pixelSize: 10
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
                onActivated: {
                    pauseButton.pausePlayScene();
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
