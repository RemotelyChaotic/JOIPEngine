#include "Settings.h"
#include "Application.h"
#include "Style.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSettings>

//----------------------------------------------------------------------------------------
//
const QString CSettings::c_sSettingContentFolder = "Content/folder";
const QString CSettings::c_sSettingFont = "Graphics/font";
const QString CSettings::c_sSettingFullscreen = "Graphics/fullscreen";
const QString CSettings::c_sSettingMuted = "Audio/muted";
const QString CSettings::c_sSettingResolution = "Graphics/resolution";
const QString CSettings::c_sSettingStyle = "Graphics/style";
const QString CSettings::c_sSettingVolume = "Audio/volume";

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
void CSettings::SetFont(const QString& sFont)
{
  QMutexLocker locker(&m_settingsMutex);
  QString sOldFont = m_spSettings->value(CSettings::c_sSettingFont).toString();

  if (sOldFont == sFont) { return; }

  m_spSettings->setValue(CSettings::c_sSettingContentFolder, sFont);

  emit FontChanged();
}

//----------------------------------------------------------------------------------------
//
QString CSettings::Font()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingFont).toString();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetFullscreen(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool bFullscreen = m_spSettings->value(CSettings::c_sSettingFullscreen).toBool();

  if (bFullscreen == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingFullscreen, bValue);

  emit FullscreenChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::Fullscreen()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingFullscreen).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetMuted(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool bMuted = m_spSettings->value(CSettings::c_sSettingMuted).toBool();

  if (bMuted == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingMuted, bValue);

  emit MutedChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::Muted()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingMuted).toBool();
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
void CSettings::SetStyle(const QString& sStyle)
{
  QMutexLocker locker(&m_settingsMutex);
  QString sOldStyle = m_spSettings->value(CSettings::c_sSettingStyle).toString();

  if (sOldStyle == sStyle) { return; }

  m_spSettings->setValue(CSettings::c_sSettingStyle, sStyle);

  emit StyleChanged();
}

//----------------------------------------------------------------------------------------
//
QString CSettings::Style()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingStyle).toString();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetVolume(double dVolume)
{
  QMutexLocker locker(&m_settingsMutex);
  double dOldValue = m_spSettings->value(CSettings::c_sSettingVolume).toDouble();

  if (qFuzzyCompare(dVolume, dOldValue)) { return; }

  m_spSettings->setValue(CSettings::c_sSettingVolume, dVolume);

  emit VolumeChanged();
}

//----------------------------------------------------------------------------------------
//
double CSettings::Volume()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingVolume).toDouble();
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
        QDir::separator() + ".." + QDir::separator() + "data";
    QFileInfo contentFileInfo(sContentPath);
    m_spSettings->setValue(CSettings::c_sSettingContentFolder,
                           contentFileInfo.absoluteFilePath());

    if (!contentFileInfo.exists())
    {
      QDir::current().mkdir("data");
    }
  }

  // check fullscreen
  if (!m_spSettings->contains(CSettings::c_sSettingFont))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingFont, "Arial");
  }

  // check fullscreen
  if (!m_spSettings->contains(CSettings::c_sSettingFullscreen))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingFullscreen, false);
  }

  // check resolution
  if (!m_spSettings->contains(CSettings::c_sSettingResolution))
  {
    bNeedsSynch = true;
    QSize size = QSize(800, 450);
    m_spSettings->setValue(CSettings::c_sSettingResolution, size);
  }

  // check muted
  if (!m_spSettings->contains(CSettings::c_sSettingMuted))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingMuted, false);
  }

  // check style
  if (!m_spSettings->contains(CSettings::c_sSettingStyle))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingStyle, "Blue Night");
  }

  // check volume
  if (!m_spSettings->contains(CSettings::c_sSettingVolume))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingVolume, 1.0);
  }

  // write file if nesseccary
  if (bNeedsSynch)
  {
    m_spSettings->sync();
  }
}
