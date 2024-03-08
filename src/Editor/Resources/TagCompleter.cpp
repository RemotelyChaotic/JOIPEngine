#include "TagCompleter.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"

CTagCompleter::CTagCompleter(QAbstractItemModel* pModel, QObject* pParent) :
    QCompleter{pModel, pParent},
    m_spCurrentProject(nullptr),
    m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}

//----------------------------------------------------------------------------------------
//
void CTagCompleter::SetCurrentProject(const tspProject& spProject)
{
  m_spCurrentProject = spProject;
}

//----------------------------------------------------------------------------------------
//
QStringList CTagCompleter::splitPath(const QString& sPath) const
{
  if (auto spDbManager = m_wpDbManager.lock())
  {
    QReadLocker locker(&m_spCurrentProject->m_rwLock);
    auto it = m_spCurrentProject->m_vspTags.find(sPath);
    if (m_spCurrentProject->m_vspTags.end() != it)
    {
      QReadLocker locker(&it->second->m_rwLock);
      return QStringList() << it->second->m_sName;
    }
    else
    {
      return sPath.split(":");
    }
  }
  else
  {
    return sPath.split(":");
  }
}
