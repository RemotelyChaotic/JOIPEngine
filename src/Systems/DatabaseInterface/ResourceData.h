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
            eLayout     = 7);

struct SResourceData
{
  SResourceData(EResourceType type = EResourceType::eOther) :
    m_type(type)
  {}
  SResourceData(const SResourceData& other) = default;

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

  constexpr static char c_sDefaultLayout[] = R"(import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import JOIP.core 1.1

Rectangle {
    id: layout
    anchors.fill: parent
    color: "transparent"

    property int spacing: 5

    readonly property bool isMobile: Settings.platform === "Android"
    property bool isLandscape: { console.log("isLandscape: " + (width > height ? "true" : "false")); return width > height; }
})";

  constexpr static char c_sScriptTypeJs[] = "js";
  constexpr static char c_sScriptTypeEos[] = "eos";
  constexpr static char c_sScriptTypeLua[] = "lua";
  constexpr static char c_sScriptTypeQml[] = "qml";
  constexpr static char c_sScriptTypeLayout[] = "layout";
  static inline const std::map<QString, SScriptDefinitionData>& DefinitionMap()
  {
    static std::map<QString, SScriptDefinitionData> c_sFileEngingDefinitionMap = {
        { c_sScriptTypeJs, {c_sScriptTypeJs, "JavaScript", "// insert code to control scene"} },
        { "json", {c_sScriptTypeJs, "JavaScript", "{\n}"}},
        { c_sScriptTypeQml, {c_sScriptTypeQml, "QML", c_sDefaultLayout}},
        { c_sScriptTypeLayout, {c_sScriptTypeQml, "QML", c_sDefaultLayout}},
        { c_sScriptTypeEos, {c_sScriptTypeEos, "JavaScript", "{\n\t\"commands\": [\n\t]\n}"} },
        { c_sScriptTypeLua, {c_sScriptTypeLua, "Lua", "-- insert code to control scene"} }
      };
    return c_sFileEngingDefinitionMap;
  }
};

#endif // RESOURCEDATA_H
