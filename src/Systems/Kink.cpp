#include "Kink.h"
#include <QCryptographicHash>
#include <QDataStream>

SKink::SKink() :
  m_rwLock(),
  m_iIdForOrdering(-1),
  m_sType(),
  m_sName(),
  m_sDescribtion()
{
}

SKink::SKink(qint32 iId, QString sType, QString sName, QString sDescribtion) :
  m_rwLock(),
  m_iIdForOrdering(iId),
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

//----------------------------------------------------------------------------------------
//
QColor CalculateKinkColor(const SKink& kink)
{
  QCryptographicHash hasher(QCryptographicHash::Md4);
  hasher.addData(kink.m_sType.toUtf8());
  QByteArray hashedArr = hasher.result();
  QDataStream ds(hashedArr);
  unsigned short r = 0;
  unsigned short g = 0;
  unsigned short b = 0;
  ds >> r >> g >> b;
  QColor hashColor(r & 0xFF, g & 0xFF, b & 0xFF);
  return hashColor;
}
