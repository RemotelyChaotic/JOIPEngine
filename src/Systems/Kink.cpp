#include "Kink.h"

SKink::SKink() :
  m_rwLock(),
  m_sType(),
  m_sName(),
  m_sDescribtion()
{
}

SKink::SKink(QString sType, QString sName, QString sDescribtion) :
  m_rwLock(),
  m_sType(sType),
  m_sName(sName),
  m_sDescribtion(sDescribtion)
{
}

SKink::SKink(const SKink& other) :
  m_rwLock(),
  m_sType(other.m_sType),
  m_sName(other.m_sName),
  m_sDescribtion(other.m_sDescribtion)
{
}

SKink::~SKink() {}
