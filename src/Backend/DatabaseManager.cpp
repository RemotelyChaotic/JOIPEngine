#include "DatabaseManager.h"
#include "Application.h"
#include "Settings.h"

CDatabaseManager::CDatabaseManager() :
  CThreadedObject(),
  m_spSettings(CApplication::Instance()->Settings())
{
}

CDatabaseManager::~CDatabaseManager()
{
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::Initialize()
{
  connect(m_spSettings.get(), &CSettings::ContentFolderChanged,
          this, &CDatabaseManager::SlotContentFolderChanged, Qt::QueuedConnection);

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::Deinitialize()
{
  disconnect(m_spSettings.get(), &CSettings::ContentFolderChanged,
          this, &CDatabaseManager::SlotContentFolderChanged);

  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::SlotContentFolderChanged()
{

}
