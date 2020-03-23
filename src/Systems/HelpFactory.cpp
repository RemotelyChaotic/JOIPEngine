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
  m_htmlHelpMap.clear();
  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CHelpFactory::Deinitialize()
{
  m_htmlHelpMap.clear();
  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
QString CHelpFactory::GetHelp(QString sKey)
{
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
  m_htmlHelpMap[sKey] = sResource;
}
