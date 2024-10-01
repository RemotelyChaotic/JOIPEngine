#ifndef RESOURCEDATA_H
#define RESOURCEDATA_H

#include "TagData.h"

#include <enum.h>
#include <QString>
#include <QUrl>

#include <map>
#include <memory>
#include <set>

BETTER_ENUM(EResourceType, qint32,
            eImage      = 0,
            eMovie      = 1,
            eSound      = 2,
            eOther      = 3,
            eScript     = 4,
            eDatabase   = 5,
            eFont       = 6,
            eLayout     = 7,
            eSequence   = 8)

struct SResourceData
{
  SResourceData(EResourceType type = EResourceType::eOther) :
    m_type(type)
  {}
  SResourceData(const SResourceData& other) = default;

  void CopyFrom(const SResourceData& other)
  {
    m_sName = other.m_sName;
    m_sPath = other.m_sPath;
    m_sSource = other.m_sSource;
    m_type = other.m_type;
    m_sResourceBundle = other.m_sResourceBundle;
    m_vsResourceTags = other.m_vsResourceTags;
  }

  QString                   m_sName;
  QUrl                      m_sPath;
  QUrl                      m_sSource;
  EResourceType             m_type = EResourceType::eOther;
  QString                   m_sResourceBundle;
  tvsTags                   m_vsResourceTags;
};

struct SScriptDefinitionData
{
  QString sType;
  QString sHighlightDefinition;
  QString sInitText;

  // as help for users just use a copy-pasta of the current layout here
  constexpr static char c_sDefaultLayout[] = R"(import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import JoipEngine 1.3
import JOIP.core 1.1
import JOIP.db 1.1

Rectangle {
    id: layout
    anchors.fill: parent
    color: "transparent"

    property int spacing: 5

    readonly property bool isMobile: Settings.platform === "Android"
    readonly property int dominantHand: Settings.dominantHand
    readonly property int iconWidth: isMobile ? 32 : 64
    readonly property int iconHeight: isMobile ? 32 : 64
    property bool isLandscape: { return width > height; }

    PlayerMediaPlayer {
        id: mediaPlayer

        anchors.fill: parent
        userName: "mediaPlayer"
        mainMediaPlayer: true

        PlayerMetronome {
            id: metronome
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            height: Math.max(parent.height * Settings.metronomeSizeRel, Settings.metronomeSizeMin)
            userName: "metronome"
        }
    }

    Rectangle {
        id: iconRect
        anchors.top: parent.top
        x: DominantHand.Left === dominantHand ? 0 : parent.width - width

        width: (parent.width - parent.spacing * 2) / 4
        height: parent.height - textBox.height;
        color: "transparent"

        PlayerIcons {
            id: icon
            width: parent.width
            height: parent.height
            iconWidth: layout.iconWidth
            iconHeight: layout.iconHeight

            anchors.right: parent.right
            anchors.bottom: parent.bottom
            userName: "icon"
        }
    }

    Rectangle {
        id: timerRect
        anchors.top: parent.top
        x: DominantHand.Left === dominantHand ? parent.width - width : 0

        width: (parent.width - parent.spacing * 2) / 4
        height: parent.height - textBox.height;
        color: "transparent"

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            width: !isMobile ? parent.width : parent.width
            height: !isMobile ? parent.height / 2 : width
            color: "transparent"

            PlayerTimer {
                id: timer
                anchors.centerIn: parent
                width: Math.min(138, parent.width)
                height: Math.min(138, parent.width)
                userName: "timer"
            }
        }
    }

    Rectangle {
        id: notificationRect

        anchors.bottom: textBox.top
        x: DominantHand.Left === dominantHand ? 0 : parent.width - width

        width: (parent.width - parent.spacing * 2) / 4
        height: (parent.height - textBox.height - layout.spacing) / 2
        color: "transparent"

        PlayerNotification {
            id: notification
            anchors.fill: parent
            userName: "notification"
        }
    }

    PlayerTextBox {
        id: textBox;

        anchors.left: parent.left
        anchors.bottom: parent.bottom

        width: parent.width
        height: parent.height / 3 - parent.spacing / 2
        iconWidth: layout.iconWidth
        iconHeight: layout.iconHeight

        userName: "textBox"
        mainTextBox: true
        displayMode: PlayerTextBox.TextBoxMode.TextBox
        hideLogAfterInactivity: true
    }

    PlayerControls {
        id: sceneControl

        x: textBox.x
        y: textBox.y - height / 2
        width: textBox.width - parent.spacing
        height: 32

        buttonHeight: 32
        buttonWidth: 48
        spacing: parent.spacing

        Button {
            id: showLogButton
            height: sceneControl.height
            Layout.preferredWidth: 48
            Layout.alignment: Qt.AlignVCenter

            text: Settings.keyBinding("Tools")
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
                source: Settings.styleFolderQml() + (textBox.logShown ?
                            "/ButtonArrowDown.svg" : "/ButtonArrowUp.svg")
            }

            onHoveredChanged: {
                if (hovered)
                {
                    soundEffects.hoverSound.play();
                }
            }
            onClicked: {
                textBox.logShown = !textBox.logShown
            }

            Shortcut {
                sequence: Settings.keyBinding("Tools")
                context: Qt.ApplicationShortcut
                onActivated: {
                    textBox.logShown = !textBox.logShown
                }
            }
        }
    }

    PlayerEventCommunicator {
        id: eventReceiver;
    }
}
)";

  constexpr static char c_sScriptTypeJs[] = "js";
  constexpr static char c_sScriptTypeEos[] = "eos";
  constexpr static char c_sScriptTypeLua[] = "lua";
  constexpr static char c_sScriptTypeQml[] = "qml";
  constexpr static char c_sScriptTypeLayout[] = "layout";
  constexpr static char c_sFileTypeSequence[] = "jseq";
  static inline const std::map<QString, SScriptDefinitionData>& DefinitionMap()
  {
    static std::map<QString, SScriptDefinitionData> c_sFileEngingDefinitionMap = {
        { c_sScriptTypeJs, {c_sScriptTypeJs, "JavaScript", "// insert code to control scene"} },
        { "json", {c_sScriptTypeJs, "JavaScript", "{\n}"}},
        { c_sScriptTypeQml, {c_sScriptTypeQml, "QML", c_sDefaultLayout}},
        { c_sScriptTypeLayout, {c_sScriptTypeQml, "QML", c_sDefaultLayout}},
        { c_sScriptTypeEos, {c_sScriptTypeEos, "JavaScript", "{\n\t\"commands\": [\n\t]\n}"} },
        { c_sScriptTypeLua, {c_sScriptTypeLua, "Lua", "-- insert code to control scene"} },
        { c_sFileTypeSequence, {c_sFileTypeSequence, "JavaScript", "{}"} }
      };
    return c_sFileEngingDefinitionMap;
  }
};

#endif // RESOURCEDATA_H
