#include "HelpFactory.h"

CHelpFactory::CHelpFactory() :
  CSystemBase()
{

}
CHelpFactory::~CHelpFactory()
{}

//----------------------------------------------------------------------------------------
//
void CHelpFactory::Initialize()
{
  QMutexLocker locker(&m_mutex);
  m_htmlHelpMap.clear();
  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CHelpFactory::Deinitialize()
{
  QMutexLocker locker(&m_mutex);
  m_htmlHelpMap.clear();
  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
QString CHelpFactory::GetHelp(QString sKey) const
{
  QMutexLocker locker(&m_mutex);
  auto it = m_htmlHelpMap.find(sKey);
  if (m_htmlHelpMap.end() != it)
  {
    return it->second;
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CHelpFactory::RegisterHelp(QString sKey, QString sResource)
{
  QMutexLocker locker(&m_mutex);
  m_htmlHelpMap[sKey] = sResource;
}

//----------------------------------------------------------------------------------------
//
std::map<QString /*sKey*/, QString /*resourcePath*/> CHelpFactory::HelpMap() const
{
  QMutexLocker locker(&m_mutex);
  return m_htmlHelpMap;
}
