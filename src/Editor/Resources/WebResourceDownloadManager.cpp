#include "WebResourceDownloadManager.h"
#include "CommonRemoteResourceAdder.h"

#include "Editor/Resources/CommandAddResource.h"

#include <QWidget>

CWebResourceDownloadManager::CWebResourceDownloadManager(QWidget* pParent)
  : QObject{pParent},
    m_spNAManager(std::make_unique<QNetworkAccessManager>()),
    m_pParent(pParent)
{
  auto spCommonAdder = std::make_shared<CCommonRemoteResourceAdder>();
  connect(spCommonAdder.get(), &CCommonRemoteResourceAdder::SignalNewResourceFile,
          this, &CWebResourceDownloadManager::SlotNewResourceFile);
  spCommonAdder->SetNetworkAccessManager(m_spNAManager.get());
  m_vspResourceAdders.push_back(spCommonAdder);
}

CWebResourceDownloadManager::~CWebResourceDownloadManager() = default;

//----------------------------------------------------------------------------------------
//
void CWebResourceDownloadManager::AddResource(const QUrl& sPath, bool bDownloadAndAddAsFile)
{
  for (const auto& spAdder : m_vspResourceAdders)
  {
    if (spAdder->CanHandleUrl(sPath))
    {
      spAdder->AddResource(sPath, bDownloadAndAddAsFile);
      return;
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CWebResourceDownloadManager::CanDownloadAndSaveAsFile(const QUrl& url) const
{
  for (const auto& spAdder : m_vspResourceAdders)
  {
    if (spAdder->CanHandleUrl(url))
    {
      return spAdder->CanDownloadAndSaveAsFile();
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CWebResourceDownloadManager::SetCurrentProject(const tspProject& spProject)
{
  m_spCurrentProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CWebResourceDownloadManager::SetUndostack(QPointer<QUndoStack> pStack)
{
  m_pUndoStack = pStack;
}

//----------------------------------------------------------------------------------------
//
void CWebResourceDownloadManager::SlotNewResourceFile(const QUrl& url, const QByteArray& ba, bool bAddAsFile)
{
  if (nullptr != m_pUndoStack)
  {
    m_pUndoStack->push(new CCommandAddResource(m_spCurrentProject, m_pParent, {{url, ba}}));
  }
}
