#ifndef RESOURCEDATA_H
#define RESOURCEDATA_H

#include <enum.h>
#include <QString>
#include <QUrl>

#include <map>
#include <memory>

BETTER_ENUM(EResourceType, qint32,
            eImage      = 0,
            eMovie      = 1,
            eSound      = 2,
            eOther      = 3,
            eScript     = 4,
            eDatabase   = 5,
            eFont       = 6);

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
};

struct SScriptDefinitionData
{
  QString sType;
  QString sHighlightDefinition;
  QString sInitText;

  constexpr static char c_sScriptTypeJs[] = "js";
  constexpr static char c_sScriptTypeEos[] = "eos";
  static inline const std::map<QString, SScriptDefinitionData>& DefinitionMap()
  {
    static std::map<QString, SScriptDefinitionData> c_sFileEngingDefinitionMap = {
        { c_sScriptTypeJs, {c_sScriptTypeJs, "JavaScript", "// insert code to control scene"} },
        { "json", {c_sScriptTypeJs, "JavaScript", "{\n}"}},
        { c_sScriptTypeEos, {c_sScriptTypeEos, "JavaScript", "{\n\t\"commands\": [\n\t]\n}"} },
      };
    return c_sFileEngingDefinitionMap;
  }
};

#endif // RESOURCEDATA_H
