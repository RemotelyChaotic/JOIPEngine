#include "TeaseStorage.h"

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
void CTeaseStorageWrapper::store(const QString& sId, const QJSValue& value)
{
  m_storage[sId] = value;
}
