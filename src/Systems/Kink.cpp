#include "Kink.h"

SKink::SKink() :
  STagData(),
  m_rwLock()
{
}

SKink::SKink(qint32 iId, QString sType, QString sName, QString sDescribtion) :
  STagData{sType, sName, sDescribtion},
  m_rwLock(),
  m_iIdForOrdering(iId)
{
}

SKink::SKink(const SKink& other) :
  STagData(other),
  m_rwLock(),
  m_iIdForOrdering(other.m_iIdForOrdering)
{
}

SKink::~SKink() {}
