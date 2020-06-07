import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtMultimedia 5.14
import JOIP.core 1.1

Rectangle {
    id: sceneControl
    color: "transparent"

    property Item soundEffects: null

    RowLayout {
        anchors.fill: parent
        spacing: 10
        layoutDirection: Qt.RightToLeft

        Button {
            id: exitButton
            Layout.preferredHeight: 48
            Layout.preferredWidth: 64
            Layout.alignment: Qt.AlignVCenter

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
            Layout.preferredHeight: 48
            Layout.preferredWidth: 64
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
