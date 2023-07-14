#ifndef TAGDATA_H
#define TAGDATA_H

#include <QString>
#include <set>

//----------------------------------------------------------------------------------------
//
struct STagData
{
  STagData() = default;
  STagData(const STagData& other) = default;

  QString                 m_sType;
  QString                 m_sName;
  QString                 m_sDescribtion;
};

typedef std::set<QString>          tvsTags;

#endif // TAGDATA_H
