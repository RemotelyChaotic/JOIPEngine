#ifndef SVERSION_H
#define SVERSION_H

#include "version.h"
#include <QRegularExpression>
#include <QString>
#include <QStringList>

struct SVersion
{
  SVersion() = default;
  SVersion(qint32 iVersion) :
    m_iMajor((iVersion & 0xff0000) >> 16),
    m_iMinor((iVersion & 0x00ff00) >> 8),
    m_iPatch(iVersion & 0x0000ff)
  {
  }
  SVersion(QString sVersion)
  {
    QStringList list = sVersion.split(QRegExp(";|,|."));
    for(qint32 i = 0; list.length() > i; ++i)
    {
      if (0 == i) { m_iMajor = list[i].toInt(); }
      if (1 == i) { m_iMinor = list[i].toInt(); }
      if (2 == i) { m_iPatch = list[i].toInt(); }
    }
  }

  qint32 m_iMajor = 0;
  qint32 m_iMinor = 0;
  qint32 m_iPatch = 0;

  bool operator ==(const SVersion& other)
  {
    return static_cast<qint32>(*this) == static_cast<qint32>(other);
  }
  bool operator !=(const SVersion& other)
  {
    return !(*this == other);
  }
  bool operator <(const SVersion& other)
  {
    return static_cast<qint32>(*this) < static_cast<qint32>(other);
  }
  bool operator >(const SVersion& other)
  {
    return static_cast<qint32>(*this) > static_cast<qint32>(other);
  }

  operator qint32() const
  {
    return QT_VERSION_CHECK(m_iMajor, m_iMinor, m_iPatch);
  }
  explicit operator QString() const
  {
    return QString("%1.%2.%3").arg(m_iMajor).arg(m_iMinor).arg(m_iPatch);
  }
};



#endif // VERSION_H
