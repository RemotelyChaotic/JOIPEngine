#include "WebResourceDownloadManager.h"
#include "CommonRemoteResourceAdder.h"

#include "Editor/Resources/CommandAddResource.h"

#include "Utils/WidgetHelpers.h"

#include <QWidget>

Q_DECLARE_METATYPE(SResourceData)

CWebResourceDownloadManager::CWebResourceDownloadManager(QWidget* pParent)
  : QObject{pParent},
    m_spNAManager(std::make_unique<QNetworkAccessManager>()),
    m_pParent(pParent)
{
  qRegisterMetaType<SResourceData>();

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
std::optional<SResourceData>
CWebResourceDownloadManager::RemoteUrlToResource(const QUrl& url,
                                                 const QByteArray& remoteFile)
{
  std::optional<SResourceData> retval = std::nullopt;

  QStringList imageFormatsList = SResourceFormats::ImageFormats();
  QStringList videoFormatsList = SResourceFormats::VideoFormats();

  qint32 iLastIndex = url.fileName().lastIndexOf('.');
  const QString sFileName = url.fileName();
  QString sFormat = "*" + sFileName.mid(iLastIndex, sFileName.size() - iLastIndex);
  if (imageFormatsList.contains(sFormat))
  {
    QPixmap mPixmap;
    mPixmap.loadFromData(remoteFile);
    if (!mPixmap.isNull())
    {
      SResourceData res;
      res.m_sName = sFileName;
      res.m_sPath = url;
      res.m_type = EResourceType::eImage;
      res.m_sResourceBundle = QString();
      res.m_sSource = url;
      retval = res;
    }
  }
  else if (videoFormatsList.contains(sFormat))
  {
    SResourceData res;
    // TODO: check video
    res.m_sName = sFileName;
    res.m_sPath = url;
    res.m_type = EResourceType::eMovie;
    res.m_sResourceBundle = QString();
    res.m_sSource = url;
    retval = res;
  }

  return retval;
}

//----------------------------------------------------------------------------------------
//
void CWebResourceDownloadManager::SlotNewResourceFile(const SResourceData& res, const QByteArray& ba, bool bAddAsFile)
{
  if (nullptr != m_pUndoStack)
  {
    SResourceData resToAdd = res;
    if (bAddAsFile)
    {
      QString sCurrentFolder = PhysicalProjectPath(m_spCurrentProject);
      QString sFolder =
        widget_helpers::GetExistingDirectory(m_pParent,
                               tr("Select directory"),
                               sCurrentFolder,
                               QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
      if (nullptr == m_pParent)
      {
        return;
      }

      if (QFileInfo::exists(sFolder))
      {
        QString sFinalPath = sFolder + "/" + res.m_sName;
        QFile f(sFinalPath);
        if (!f.open(QIODevice::ReadWrite))
        {
          qWarning() << tr("Could not open %1 for writing.").arg(sFinalPath);
        }

        f.write(ba);

        resToAdd.m_sPath = joip_resource::CreatePathFromAbsoluteUrl(sFinalPath, m_spCurrentProject);
      }
      else
      {
        qWarning() << tr("Folder %1 does not exist.").arg(sFolder);
      }
    }

    m_pUndoStack->push(new CCommandAddResource(m_spCurrentProject, m_pParent, {{resToAdd.m_sName, resToAdd}}));
  }
}
