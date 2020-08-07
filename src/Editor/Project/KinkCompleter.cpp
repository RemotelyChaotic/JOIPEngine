#include "KinkCompleter.h"
#include "Application.h"
#include "KinkTreeModel.h"
#include "Systems/DatabaseManager.h"

//----------------------------------------------------------------------------------------
//
CKinkCompleter::CKinkCompleter(CKinkTreeModel* pModel, QObject* pParent) :
  QCompleter(pModel, pParent),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}

//----------------------------------------------------------------------------------------
//
QStringList CKinkCompleter::splitPath(const QString& sPath) const
{
  QString sReultingKink = sPath;
  if (auto spDbManager = m_wpDbManager.lock())
  {
    tspKink spKink = spDbManager->FindKink(sPath);
    if (nullptr != spKink)
    {
      QReadLocker locker(&spKink->m_rwLock);
      return QStringList() << spKink->m_sType << spKink->m_sName;
    }
    else
    {
      return sPath.split("/");
    }
  }
  else
  {
    return sPath.split("/");
  }
}

//----------------------------------------------------------------------------------------
//
QString CKinkCompleter::pathFromIndex(const QModelIndex& index) const
{
  if (!index.isValid()) { return QString(); }

  CKinkTreeModel* pModel = dynamic_cast<CKinkTreeModel*>(model());
  if (nullptr == pModel) { return QString(); }

  QString sData = pModel->data(index, completionRole()).toString();

  QModelIndex idx = index;
  QStringList vsList;
  do {
      QString sData = pModel->data(index, completionRole()).toString();
      vsList.prepend(sData);
      QModelIndex parent = idx.parent();
      idx = parent.sibling(parent.row(), index.column());
  } while (idx.isValid());

  return vsList.last();
}
