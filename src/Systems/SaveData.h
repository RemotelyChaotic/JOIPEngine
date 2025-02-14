#ifndef SSAVEDATA_H
#define SSAVEDATA_H

#include "ISerializable.h"
#include "DatabaseInterface/SaveData.h"
#include <QReadWriteLock>
#include <QJSValue>
#include <memory>

struct SProject;

namespace save_data
{
  [[maybe_unused]] const char c_sFileAchievements[] = "achievements";
  [[maybe_unused]] const char c_sFileStats[] = "stats";
  [[maybe_unused]] const char c_sSaveFileEnding[] = "joipsave";

  [[maybe_unused]] const char c_sFileStatNumPlayed[] = "numPlayed";
  [[maybe_unused]] const char c_sFileStatNumFinished[] = "numFinished";
  [[maybe_unused]] const char c_sFileStatPlayTime[] = "playTime";
}

struct SSaveData : public ISerializable, std::enable_shared_from_this<SSaveData>,
                   public SSaveDataData
{
  SSaveData();
  SSaveData(const SSaveData& other);
  SSaveData(QString sName, QString sDescribtion, ESaveDataType type,
            QString sResource, QVariant data);
  SSaveData& operator=(const SSaveData& other);
  ~SSaveData() override;

  mutable QReadWriteLock    m_rwLock;
  std::shared_ptr<SProject> m_spParent = nullptr;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;

  std::shared_ptr<SSaveData> GetPtr()
  {
    return shared_from_this();
  }
};

typedef std::shared_ptr<SSaveData>      tspSaveData;
typedef std::map<QString, tspSaveData>  tspSaveDataMap;

Q_DECLARE_METATYPE(tspSaveData)

namespace save_data
{
  QJsonObject ToJsonObject(QVariant data);
  QJsonObject ToJsonObject(QJSValue data);
  QVariant FromJsonObject(const QJsonObject& json);
}

#endif // SSAVEDATA_H
