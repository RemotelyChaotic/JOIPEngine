#include "TeaseStorage.h"
#include "ProjectSavegameManager.h"

#include <QQmlContext>
#include <QQmlEngine>

CTeaseStorageWrapper::CTeaseStorageWrapper(QObject* pParent) :
  QObject(pParent)
{
}
CTeaseStorageWrapper::~CTeaseStorageWrapper()
{
  clear();
}

//----------------------------------------------------------------------------------------
//
CProjectSavegameManager* CTeaseStorageWrapper::GetSaveManager() const
{
  return m_pSaveManager;
}

//----------------------------------------------------------------------------------------
//
void CTeaseStorageWrapper::SetSaveManager(CProjectSavegameManager* pManager)
{
  if (m_pSaveManager != pManager)
  {
    m_pSaveManager = pManager;
    emit saveManagerChanged();
  }
}

//----------------------------------------------------------------------------------------
//
void CTeaseStorageWrapper::clear()
{
  m_storage.clear();
}

//----------------------------------------------------------------------------------------
//
QJSValue CTeaseStorageWrapper::load(const QString& sId)
{
  auto it = m_storage.find(sId);
  if (m_storage.end() != it)
  {
    return it->second;
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QJSValue CTeaseStorageWrapper::load(const QString& sId, const QString& sContext)
{
  if (sContext.isEmpty())
  {
    return load(sId);
  }
  else if (nullptr != m_pSaveManager)
  {
    QQmlContext* pCurrentContext = QQmlEngine::contextForObject(this);
    QQmlEngine* pEngine = pCurrentContext->engine();
    return pEngine->toScriptValue(m_pSaveManager->load(sId, sContext));
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
void CTeaseStorageWrapper::store(const QString& sId, const QJSValue& value)
{
  m_storage[sId] = value;
}

//----------------------------------------------------------------------------------------
//
void CTeaseStorageWrapper::store(const QString& sId, const QJSValue& value,
                                 const QString& sContext)
{
  if (sContext.isEmpty())
  {
    store(sId, value);
  }
  else if (nullptr != m_pSaveManager)
  {
    m_pSaveManager->store(sId, value.toVariant(), sContext);
  }
}

//----------------------------------------------------------------------------------------
//
void CTeaseStorageWrapper::loadPersistent(const QString& sId, const QString& sContext)
{
  if (nullptr != m_pSaveManager)
  {
    const QString sFile = sContext.isEmpty() ? "savefile" : sContext;
    QQmlContext* pCurrentContext = QQmlEngine::contextForObject(this);
    QQmlEngine* pEngine = pCurrentContext->engine();
    QJSValue val = pEngine->toScriptValue(m_pSaveManager->load(sId, sFile));
    if (!val.isNull() && !val.isUndefined())
    {
      store(sId, val);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTeaseStorageWrapper::storePersistent(const QString& sId, const QString& sContext)
{
  QJSValue val = load(sId);
  if (!val.isNull() && !val.isUndefined() && nullptr != m_pSaveManager)
  {
    const QString sFile = sContext.isEmpty() ? "savefile" : sContext;
    m_pSaveManager->store(sId, val.toVariant(), sFile);
  }
}
