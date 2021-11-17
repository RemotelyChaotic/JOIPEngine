#include "TeaseStorage.h"

CTeaseStorage::CTeaseStorage(QObject* pParent) :
  QObject(pParent)
{
}
CTeaseStorage::~CTeaseStorage()
{
  clear();
}

//----------------------------------------------------------------------------------------
//
void CTeaseStorage::clear()
{
  m_storage.clear();
}

//----------------------------------------------------------------------------------------
//
QJSValue CTeaseStorage::load(const QString& sId)
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
void CTeaseStorage::store(const QString& sId, const QJSValue& value)
{
  m_storage[sId] = value;
}
