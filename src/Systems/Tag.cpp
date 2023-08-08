#include "Tag.h"

#include <QCryptographicHash>
#include <QDataStream>

STag::STag() :
  SLockableTagData()
{
}

STag::STag(QString sType, QString sName, QString sDescribtion) :
    SLockableTagData{sType, sName, sDescribtion}
{
}

STag::STag(const STag& other) :
    SLockableTagData(other),
    m_spParent(other.m_spParent),
    m_vsResourceRefs(other.m_vsResourceRefs)
{
}

STag::~STag() {}

//----------------------------------------------------------------------------------------
//
QJsonObject STag::ToJsonObject()
{
  QWriteLocker locker(&m_rwLock);
  return {
    { "sType", m_sType },
    { "sName", m_sName },
    { "sDescribtion", m_sDescribtion }
  };
}

//----------------------------------------------------------------------------------------
//
void STag::FromJsonObject(const QJsonObject& json)
{
  QWriteLocker locker(&m_rwLock);
  auto it = json.find("sType");
  if (it != json.end())
  {
    m_sType = it.value().toString();
  }
  it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
  it = json.find("sDescribtion");
  if (it != json.end())
  {
    m_sDescribtion = it.value().toString();
  }
}

//----------------------------------------------------------------------------------------
//
QColor CalculateTagColor(const STagData& tag)
{
  QCryptographicHash hasher(QCryptographicHash::Md4);
  hasher.addData(tag.m_sType.toUtf8());
  QByteArray hashedArr = hasher.result();
  QDataStream ds(hashedArr);
  unsigned short r = 0;
  unsigned short g = 0;
  unsigned short b = 0;
  ds >> r >> g >> b;
  QColor hashColor(r & 0xFF, g & 0xFF, b & 0xFF);
  return hashColor;
}
