#include "ProjectSavegameManager.h"
#include "Application.h"
#include "Settings.h"

#include "Systems/Database/SaveData.h"
#include "Systems/Script/ScriptDbWrappers.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>

//----------------------------------------------------------------------------------------
//
class CProjectSavegameFSPersistance : public IProjectSavegamePersistance
{
public:
  CProjectSavegameFSPersistance() :
    m_sBasePath(QFileInfo(CApplication::Instance()->Settings()->FileName()).absolutePath())
  {}
  ~CProjectSavegameFSPersistance() = default;

  //--------------------------------------------------------------------------------------
  //
  std::optional<SSettingFile> Read(const QString& sProject, const QString& sContext) const override
  {
    QFile f(m_sBasePath + "/" + sProject + "/" + sContext + "." + save_data::c_sSaveFileEnding);
    if (f.exists() && f.open(QIODevice::ReadOnly))
    {
      QJsonParseError err;
      QByteArray arr = f.readAll();
      QJsonDocument doc = QJsonDocument::fromJson(arr, &err);
      if (QJsonParseError::NoError == err.error)
      {
        SSettingFile fileOut;
        fileOut.m_sResourceName = sContext;

        auto obj = doc.object();
        for (auto it = obj.begin(); obj.end() != it; ++it)
        {
          const QString sId = it.key();
          QVariant var = save_data::FromJsonObject(it.value().toObject());
          fileOut.m_settings[sId] = var;
        }

        return fileOut;
      }
      else
      {
        qWarning() <<
            QObject::tr("Error reading save file %1: %2")
                          .arg(f.fileName()).arg(err.errorString());
      }
    }
    return std::nullopt;
  }

  //--------------------------------------------------------------------------------------
  //
  void Write(const QString& sProject, const QString& sContext, const SSettingFile& file) const override
  {
    QDir d(m_sBasePath + "/" + sProject);
    if (!d.exists())
    {
      bool bOk = QDir(m_sBasePath).mkdir(sProject);
      if (!bOk)
      {
        qWarning() <<
            QObject::tr("Error creating save folder %1: %2").arg(d.absolutePath());
      }
    }

    QSaveFile f(m_sBasePath + "/" + sProject + "/" + sContext + "." + save_data::c_sSaveFileEnding);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
      QJsonObject objRoot;
      for (auto it = file.m_settings.begin(); file.m_settings.end() != it; ++it)
      {
        QJsonObject obj = save_data::ToJsonObject(it.value());
        objRoot[it.key()] = obj;
      }

      QJsonDocument doc(objRoot);
      f.write(doc.toJson());

      f.commit();
    }
    else
    {
      qWarning() <<
          QObject::tr("Error writing save file %1: %2")
              .arg(f.fileName()).arg(f.errorString());
    }
  }

private:
  QString m_sBasePath;
};

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IProjectSavegamePersistance> GetPersistance()
{
  // todo: if other persistance needed, create new instances here
  return std::make_shared<CProjectSavegameFSPersistance>();
}

//----------------------------------------------------------------------------------------
//
CProjectSavegameManager::CProjectSavegameManager(QObject* pParent) :
  QObject{pParent},
  m_spPersistence(GetPersistance())
{
}
CProjectSavegameManager::~CProjectSavegameManager() = default;

//----------------------------------------------------------------------------------------
//
CProjectScriptWrapper* CProjectSavegameManager::Project() const { return m_pProject; }
void CProjectSavegameManager::SetProject(CProjectScriptWrapper* pProj)
{
  if (m_pProject != pProj)
  {
    m_pProject = pProj;
    AutoloadSaves();
    emit projectChanged();
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectSavegameManager::load(const QString& sId, const QString& sContext)
{
  using SSettingFile = IProjectSavegamePersistance::SSettingFile;
  if (nullptr == m_pProject || nullptr == m_pProject->Data())
  {
    qWarning() << tr("Save Manager load without project.");
    return QVariant();
  }

  auto it = m_vSettings.find(sContext);
  if (m_vSettings.end() != it)
  {
    auto itVal = it->second.m_settings.find(sId);
    if (it->second.m_settings.end() != itVal)
    {
      return itVal.value();
    }
  }
  else
  {
    auto spData = m_pProject->Data();
    QReadLocker l(&spData->m_rwLock);

    std::optional<SSettingFile> optFile =
        m_spPersistence->Read(spData->m_sName, sContext);
    if (optFile.has_value())
    {
      SSettingFile file = optFile.value();
      m_vSettings[sContext] = file;

      auto itVal = file.m_settings.find(sId);
      if (file.m_settings.end() != itVal)
      {
        return itVal.value();
      }
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
void CProjectSavegameManager::removeData(const QString& sId, const QString& sContext)
{
  if (nullptr == m_pProject || nullptr == m_pProject->Data())
  {
    qWarning() << tr("Save Manager removeData without project.");
    return;
  }

  auto it = m_vSettings.find(sContext);
  if (m_vSettings.end() != it)
  {
    auto itSett = it->second.m_settings.find(sId);
    if (it->second.m_settings.end() != itSett)
    {
      it->second.m_settings.erase(itSett);
    }

    auto spData = m_pProject->Data();
    QReadLocker l(&spData->m_rwLock);

    m_spPersistence->Write(spData->m_sName, sContext, it->second);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectSavegameManager::store(const QString& sId, QVariant value, const QString& sContext)
{
  using SSettingFile = IProjectSavegamePersistance::SSettingFile;
  if (nullptr == m_pProject || nullptr == m_pProject->Data())
  {
    qWarning() << tr("Save Manager store without project.");
    return;
  }

  auto it = m_vSettings.find(sContext);
  if (m_vSettings.end() == it)
  {
    m_vSettings[sContext] = SSettingFile{sContext, {}};
    it = m_vSettings.find(sContext);
  }

  QVariant oldVal = it->second.m_settings[sId];
  it->second.m_settings[sId] = value;

  {
    auto spData = m_pProject->Data();
    QReadLocker l(&spData->m_rwLock);

    m_spPersistence->Write(spData->m_sName, sContext, it->second);
  }

  if (save_data::c_sFileAchievements == sContext)
  {
    emit achievementValueChanged(sId, value, oldVal);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectSavegameManager::AutoloadSaves()
{
  using SSettingFile = IProjectSavegamePersistance::SSettingFile;

  if (nullptr != m_pProject)
  {
    auto spData = m_pProject->Data();
    QReadLocker l(&spData->m_rwLock);

    // read achievements
    {
      std::optional<SSettingFile> optAchs =
          m_spPersistence->Read(spData->m_sName, save_data::c_sFileAchievements);
      SSettingFile achs = optAchs.value_or(SSettingFile{save_data::c_sFileAchievements, {}});
      for (const auto& [sName, spAch] : spData->m_vspAchievements)
      {
        auto it = achs.m_settings.find(sName);
        if (achs.m_settings.end() == it)
        {
          QReadLocker lA(&spAch->m_rwLock);
          switch (spAch->m_type)
          {
            case ESaveDataType::eBool:
              achs.m_settings[sName] = false;
              break;
            case ESaveDataType::eInt:
              achs.m_settings[sName] = 0;
              break;
            default: break;
          }
        }
      }
      m_vSettings[save_data::c_sFileAchievements] = achs;
    }

    // read stats
    {
      std::optional<SSettingFile> optStats =
          m_spPersistence->Read(spData->m_sName, save_data::c_sFileStats);
      m_vSettings[save_data::c_sFileStats] =
          optStats.value_or(SSettingFile{save_data::c_sFileStats, {}});
    }
  }
}
