#include "CApplication.h"
#include "CSettings.h"
#include <QDir>
#include <QFileInfo>
#include <QSettings>

CApplication::CApplication(int argc, char *argv[]) :
  QGuiApplication(argc, argv),
  m_spSettings(nullptr),
  m_bInitialized(false)
{

}

CApplication::~CApplication()
{

}

//----------------------------------------------------------------------------------------
//
void CApplication::Initialize()
{
  m_spSettings = std::make_shared<QSettings>(QSettings::IniFormat, QSettings::UserScope,
    CSettings::c_sOrganisation, CSettings::c_sApplicationName);

  GenerateSettingsIfNotExists();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CApplication::GenerateSettingsIfNotExists()
{
  bool bNeedsSynch = false;

  // check content path
  if (!m_spSettings->contains(CSettings::c_sSettingContentFolder))
  {
    bNeedsSynch = true;
    QString sContentPath = applicationDirPath() + QDir::separator() + "data";
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
