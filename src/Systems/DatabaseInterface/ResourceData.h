#ifndef RESOURCEDATA_H
#define RESOURCEDATA_H

#include <enum.h>
#include <QString>
#include <QUrl>
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
};

#endif // RESOURCEDATA_H
