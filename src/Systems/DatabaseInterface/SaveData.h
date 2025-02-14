#ifndef SAVEDATA_H
#define SAVEDATA_H

#include <enum.h>

#include <QString>
#include <QVariant>

BETTER_ENUM(ESaveDataType, qint32,
            eBool = 0,
            eInt,
            eDouble,
            eString,
            eRegexp,
            eDate,
            eUrl, // reserved for Qt6
            eArray,
            eObject,
            eNull)

//----------------------------------------------------------------------------------------
//
struct SSaveDataData
{
  SSaveDataData() {}
  SSaveDataData(const SSaveDataData& other) :
      m_sName(other.m_sName), m_sDescribtion(other.m_sDescribtion), m_type(other.m_type),
      m_sResource(other.m_sResource), m_data(other.m_data) {}
  SSaveDataData(QString sName, QString sDescribtion, ESaveDataType type,
            QString sResource, QVariant data) :
      m_sName(sName), m_sDescribtion(sDescribtion), m_type(type),
      m_sResource(sResource), m_data(data) {}
  SSaveDataData& operator=(const SSaveDataData& other) {
    m_sName = other.m_sName;
    m_sDescribtion = other.m_sDescribtion;
    m_type = other.m_type;
    m_sResource = other.m_sResource;
    m_data = other.m_data;
    return *this;
  }
  virtual ~SSaveDataData() = default;

  QString                 m_sName;
  QString                 m_sDescribtion;
  ESaveDataType           m_type = ESaveDataType::eNull;
  QString                 m_sResource;
  QVariant                m_data;
};

#endif // SAVEDATA_H
