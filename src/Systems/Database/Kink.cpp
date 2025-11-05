#include "Kink.h"

SKink::SKink() :
  SLockableTagData()
{
}

SKink::SKink(qint32 iId, QString sType, QString sName, QString sDescribtion) :
  SLockableTagData{sType, sName, sDescribtion},
  m_iIdForOrdering(iId)
{
}

SKink::SKink(const SKink& other) :
  SLockableTagData(other),
  m_iIdForOrdering(other.m_iIdForOrdering)
{
}

SKink::~SKink() {}
