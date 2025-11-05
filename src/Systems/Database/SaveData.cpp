#include "SaveData.h"

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QJsonArray>
#include <QRegExp>
#include <QRegularExpression>
#include <QUrl>

namespace
{
  const char c_sValueProp[] = "value";
  const char c_sTypeProp[] = "type";
  const char c_sKeyProp[] = "key";
}

//----------------------------------------------------------------------------------------
//
SSaveData::SSaveData() :
  SSaveDataData(),
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent()
{
}
SSaveData::SSaveData(const SSaveData& other) :
  SSaveDataData(other),
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent(other.m_spParent)
{

}
SSaveData::SSaveData(QString sName, QString sDescribtion, ESaveDataType type,
          QString sResource, QVariant data) :
  SSaveDataData(sName, sDescribtion, type, sResource, data),
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent()
{
}
SSaveData& SSaveData::operator=(const SSaveData& other)
{
  SSaveDataData::operator=(other);
  m_spParent = other.m_spParent;
  return *this;
}
SSaveData::~SSaveData() {
  int i = 0;
  Q_UNUSED(i)
};

//----------------------------------------------------------------------------------------
//
QJsonObject SSaveData::ToJsonObject()
{
  QWriteLocker locker(&m_rwLock);
  return {
      { "sName", m_sName },
      { "sDescribtion", m_sDescribtion },
      { c_sTypeProp, m_type._value },
      { "sResource", m_sResource },
      { "data", save_data::ToJsonObject(m_data) }
  };
}

//----------------------------------------------------------------------------------------
//
void SSaveData::FromJsonObject(const QJsonObject& json)
{
  QWriteLocker locker(&m_rwLock);
  auto it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
  it = json.find("sDescribtion");
  if (it != json.end())
  {
    m_sDescribtion = it.value().toString();
  }
  it = json.find(c_sTypeProp);
  if (it != json.end())
  {
    qint32 iValue = it.value().toInt();
    if (ESaveDataType::_size() > static_cast<size_t>(iValue) && -1 < iValue)
    {
      m_type = ESaveDataType::_from_integral(iValue);
    }
  }
  it = json.find("sResource");
  if (it != json.end())
  {
    m_sResource = it.value().toString();
  }
  it = json.find("data");
  if (it != json.end())
  {
    m_data = save_data::FromJsonObject(it.value().toObject());
  }
}

//----------------------------------------------------------------------------------------
//
QJsonObject save_data::ToJsonObject(QVariant data)
{
  QJsonObject val;
  switch(data.type())
  {
    case QVariant::Bool:
    {
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eBool}, { c_sValueProp, data.toBool()} };
    } break;
    case QVariant::Int:
    {
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eInt}, { c_sValueProp, data.toInt()} };
    } break;
    case QVariant::LongLong:
    {
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eDouble}, { c_sValueProp, data.toLongLong()} };
    } break;
    case QVariant::Double:
    {
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eDouble}, { c_sValueProp, data.toDouble()} };
    } break;
    case QVariant::String:
    {
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eString}, { c_sValueProp, data.toString()} };
    } break;
    case QVariant::RegExp:
    {
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eRegexp}, { c_sValueProp, data.toRegExp().pattern()} };
    } break;
    case QVariant::RegularExpression:
    {
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eRegexp}, { c_sValueProp, data.toRegularExpression().pattern()} };
    } break;
    case QVariant::Date:
    {
      QDateTime dt = data.toDateTime();
      QByteArray arr;
      {
        QDataStream str(&arr, QIODevice::ReadWrite | QIODevice::Truncate);
        str << dt;
      }
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eDate}, { c_sValueProp, QString::fromUtf8(arr.toBase64())} };
    } break;
    case QVariant::Url:
    {
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eRegexp}, { c_sValueProp, data.toUrl().toString()} };
    } break;
    case QVariant::List:
    {
      QJsonArray arr;
      QVariantList list = data.toList();
      const qint32 iLength = list.length();
      for (qint32 i = 0; i < iLength; ++i)
      {
        QVariant element = list[i];
        arr << ToJsonObject(element);
      }
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eArray}, { c_sValueProp, arr} };
    } break;
    case QVariant::Map:
    {
      QJsonArray arr;
      QVariantMap map = data.toMap();
      for (auto it = map.begin(); map.end() != it; ++it)
      {
        QJsonObject obj = ToJsonObject(it.value());
        obj[c_sKeyProp] = it.key();
        arr << obj;
      }
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eObject}, { c_sValueProp, arr} };
    } break;
    case QVariant::Invalid:
    {
      val = QJsonObject { {c_sTypeProp, ESaveDataType::eNull} };
    }
    default: break;
  }

  return val;
}

//----------------------------------------------------------------------------------------
//
QJsonObject save_data::ToJsonObject(QJSValue data)
{
  QJsonObject val;
  if (data.isBool())
  {
    val = QJsonObject { {c_sTypeProp, ESaveDataType::eBool}, { c_sValueProp, data.toBool()} };
  }
  else if (data.isNumber())
  {
    val = QJsonObject { {c_sTypeProp, ESaveDataType::eDouble}, { c_sValueProp, data.toNumber()} };
  }
  else if (data.isString())
  {
    val = QJsonObject { {c_sTypeProp, ESaveDataType::eString}, { c_sValueProp, data.toString()} };
  }
  else if (data.isRegExp())
  {
    val = QJsonObject { {c_sTypeProp, ESaveDataType::eRegexp}, { c_sValueProp, data.toString()} };
  }
  else if (data.isDate())
  {
    QDateTime dt = data.toDateTime();
    QByteArray arr;
    {
      QDataStream str(&arr, QIODevice::ReadWrite | QIODevice::Truncate);
      str << dt;
    }
    val = QJsonObject { {c_sTypeProp, ESaveDataType::eDate}, { c_sValueProp, QString::fromUtf8(arr.toBase64())} };
  }
  /*
  else if (data.isUrl())
  {
    // reserved for Qt6
  }
  */
  else if (data.isArray())
  {
    QJsonArray arr;
    const qint32 iLength = data.property("length").toInt();
    for (qint32 i = 0; i < iLength; ++i)
    {
      QJSValue element = data.property(static_cast<quint32>(i));
      arr << ToJsonObject(element);
    }
    val = QJsonObject { {c_sTypeProp, ESaveDataType::eArray}, { c_sValueProp, arr} };
  }
  else if (data.isObject())
  {
    val = ToJsonObject(data.toVariant());
  }
  else if (data.isNull() || data.isUndefined())
  {
    val = QJsonObject { {c_sTypeProp, ESaveDataType::eNull} };
  }
  return val;
}

//----------------------------------------------------------------------------------------
//
QVariant save_data::FromJsonObject(const QJsonObject& json)
{
  QVariant var;
  auto it = json.find(c_sTypeProp);
  if (json.end() != it)
  {
    qint32 iTypeValue = it.value().toInt();
    if (ESaveDataType::_size() > static_cast<size_t>(iTypeValue) && -1 < iTypeValue)
    {
      auto type = ESaveDataType::_from_integral(iTypeValue);
      it = json.find(c_sValueProp);
      if (json.end() != it)
      {
        switch (type)
        {
          case ESaveDataType::eBool:
          {
            var = it->toBool(false);
          } break;
          case ESaveDataType::eInt:
          {
            var = it->toInt(0);
          } break;
          case ESaveDataType::eDouble:
          {
            var = it->toDouble(0.0);
          } break;
          case ESaveDataType::eString:
          {
            var = it->toString();
          } break;
          case ESaveDataType::eRegexp:
          {
            var = QRegExp(it->toString());
          } break;
          case ESaveDataType::eDate:
          {
            QDateTime dt;
            QString sb64 = it->toString();
            QByteArray arr = QByteArray::fromBase64(sb64.toUtf8());
            {
              QDataStream str(&arr, QIODevice::ReadOnly);
              str >> dt;
            }
            var = dt;
          } break;
          case ESaveDataType::eUrl:
          {
            var = QUrl(it->toString());
          } break;
          case ESaveDataType::eArray:
          {
            QVariantList list;
            QJsonArray arr = it->toArray();
            for (qint32 i = 0; arr.size() > i; ++i)
            {
              QJsonValue val = arr.at(i);
              list << FromJsonObject(val.toObject());
            }
            var = list;
          } break;
          case ESaveDataType::eObject:
          {
            QVariantMap map;
            QJsonArray arr = it->toArray();
            for (qint32 i = 0; arr.size() > i; ++i)
            {
              QJsonValue val = arr.at(i);
              QJsonObject obj = val.toObject();
              auto itKey = obj.find(c_sKeyProp);
              if (json.end() != it)
              {
                QString sKey = itKey.value().toString();
                QVariant val = FromJsonObject(obj);
                map.insert(sKey, val);
              }
            }
            var = map;
          } break;
          case ESaveDataType::eNull: [[fallthrough]];
          default: break;
        }
      }
    }
  }
  return var;
}
