#include "Settings.h"
#include "Application.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSettings>

//----------------------------------------------------------------------------------------
//
const QString CSettings::c_sSettingResolution = "Graphics/resolution";
const QString CSettings::c_sSettingContentFolder = "Content/folder";

const QString CSettings::c_sOrganisation = "Private";
const QString CSettings::c_sApplicationName = "JOIPEngine";

//----------------------------------------------------------------------------------------
//
CSettings::CSettings(QObject* pParent) :
  QObject (pParent),
  m_spSettings(std::make_shared<QSettings>(QSettings::IniFormat, QSettings::UserScope,
     CSettings::c_sOrganisation, CSettings::c_sApplicationName))
{
  GenerateSettingsIfNotExists();
}

//----------------------------------------------------------------------------------------
//
CSettings::~CSettings()
{

}

//----------------------------------------------------------------------------------------
//
void CSettings::SetContentFolder(const QString& sPath)
{
  QMutexLocker locker(&m_settingsMutex);
  QString sOldPath = m_spSettings->value(CSettings::c_sSettingContentFolder).toString();
  QFileInfo contentFileInfo(sPath);

  if (!contentFileInfo.exists() || sOldPath == sPath) { return; }

  m_spSettings->setValue(CSettings::c_sSettingContentFolder, sPath);

  emit ContentFolderChanged();
}

//----------------------------------------------------------------------------------------
//
QString CSettings::ContentFolder()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingContentFolder).toString();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetResolution(const QSize& size)
{
  QMutexLocker locker(&m_settingsMutex);
  QSize oldValue = m_spSettings->value(CSettings::c_sSettingResolution).toSize();

  if (size == oldValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingResolution, size);

  emit ResolutionChanged();
}

//----------------------------------------------------------------------------------------
//
QSize CSettings::Resolution()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingResolution).toSize();
}

//----------------------------------------------------------------------------------------
//
void CSettings::GenerateSettingsIfNotExists()
{
  QMutexLocker locker(&m_settingsMutex);
  bool bNeedsSynch = false;

  // check content path
  if (!m_spSettings->contains(CSettings::c_sSettingContentFolder))
  {
    bNeedsSynch = true;
    QString sContentPath = QCoreApplication::instance()->applicationDirPath() +
        QDir::separator() + "data";
    QFileInfo contentFileInfo(sContentPath);
    m_spSettings->setValue(CSettings::c_sSettingContentFolder,
                           contentFileInfo.absoluteFilePath());

    if (!contentFileInfo.exists())
    {
      QDir::current().mkdir("data");
    }
  }

  // check resolution
  if (!m_spSettings->contains(CSettings::c_sSettingResolution))
  {
    bNeedsSynch = true;
    QSize size = QSize(800, 450);
    m_spSettings->setValue(CSettings::c_sSettingResolution, size);
  }

  // write file if nesseccary
  if (bNeedsSynch)
  {
    m_spSettings->sync();
  }
}
