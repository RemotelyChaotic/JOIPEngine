#ifndef TAGDATA_H
#define TAGDATA_H

#include <QReadWriteLock>
#include <QString>
#include <set>

//----------------------------------------------------------------------------------------
//
struct STagData
{
  STagData() {}
  STagData(const STagData& other) :
      m_sType(other.m_sType), m_sName(other.m_sName), m_sDescribtion(other.m_sDescribtion) {}
  STagData(QString sType, QString sName, QString sDescribtion) :
      m_sType(sType), m_sName(sName), m_sDescribtion(sDescribtion) {}
  virtual ~STagData() = default;

  QString                 m_sType;
  QString                 m_sName;
  QString                 m_sDescribtion;
};

//----------------------------------------------------------------------------------------
//
struct SLockableTagData : public STagData
{
  SLockableTagData() : STagData(), m_rwLock(QReadWriteLock::Recursive) {}
  SLockableTagData(QString sType, QString sName, QString sDescribtion) :
      STagData(sType, sName, sDescribtion), m_rwLock(QReadWriteLock::Recursive) {}
  SLockableTagData(const SLockableTagData& other) :
      STagData(other), m_rwLock(QReadWriteLock::Recursive) {}
  ~SLockableTagData() override = default;

  mutable QReadWriteLock  m_rwLock;
};

typedef std::set<QString>          tvsTags;

#endif // TAGDATA_H
