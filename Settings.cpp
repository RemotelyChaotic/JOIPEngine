#include "Settings.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

//----------------------------------------------------------------------------------------
//
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
  return m_spSettings->value(CSettings::c_sSettingContentFolder).toString();
}

//----------------------------------------------------------------------------------------
//
void CSettings::GenerateSettingsIfNotExists()
{
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

  // write file if nesseccary
  if (bNeedsSynch)
  {
    m_spSettings->sync();
  }
}
