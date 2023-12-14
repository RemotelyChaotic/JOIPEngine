#include "CommandAddResource.h"
#include "Application.h"
#include "Editor/EditorCommandIds.h"
#include "Settings.h"
#include "Systems/DatabaseManager.h"
#include "Systems/PhysFs/PhysFsFileEngine.h"
#include "Systems/Resource.h"

#include <QDebug>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>

namespace  {
//----------------------------------------------------------------------------------------
//
  tspResourceMap AddFilesToProjectResources(std::shared_ptr<SProject> spCurrentProject,
                                            std::shared_ptr<CSettings> spSettings,
                                            std::weak_ptr<CDatabaseManager> wpDbManager,
                                            QPointer<QWidget> pParentForDialog,
                                            const QStringList& vsFiles)
  {
    if (nullptr == spCurrentProject) { return tspResourceMap(); }

    // add file to respective category
    bool bAddedFiles = false;
    QStringList vsNeedsToMove;
    tspResourceMap retMap;
    const QDir projectDir = PhysicalProjectPath(spCurrentProject);
    for (QString sFileName : vsFiles)
    {
      QFileInfo info(sFileName);
      const QString sTest = info.absoluteFilePath();
      if (!info.absoluteFilePath().contains(projectDir.absolutePath()) &&
          !sFileName.startsWith(CPhysFsFileEngineHandler::c_sScheme))
      {
        vsNeedsToMove.push_back(sFileName);
      }
      else
      {
        QString sRelativePath = projectDir.relativeFilePath(sFileName);
        QUrl url = ResourceUrlFromLocalFile(sRelativePath);
        const QString sEnding = "*." + info.suffix();
        auto spDbManager = wpDbManager.lock();
        if (nullptr != spDbManager)
        {
          QString sResource;
          if (SResourceFormats::ImageFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, url, EResourceType::eImage);
            bAddedFiles = true;
          }
          else if (SResourceFormats::VideoFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, url, EResourceType::eMovie);
            bAddedFiles = true;
          }
          else if (SResourceFormats::AudioFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, url, EResourceType::eSound);
            bAddedFiles = true;
          }
          else if (SResourceFormats::OtherFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, url, EResourceType::eOther);
            bAddedFiles = true;
          }
          else if (SResourceFormats::ScriptFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, url, EResourceType::eScript);
            bAddedFiles = true;
          }
          else if (SResourceFormats::DatabaseFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, url, EResourceType::eDatabase);
            bAddedFiles = true;
          }
          else if (SResourceFormats::FontFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, url, EResourceType::eFont);
            bAddedFiles = true;
          }
          else if (SResourceFormats::LayoutFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, url, EResourceType::eLayout);
            bAddedFiles = true;
          }
          else if (SResourceFormats::SequenceFormat().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, url, EResourceType::eSequence);
            bAddedFiles = true;
          }
          else if (SResourceFormats::ArchiveFormats().contains(sEnding))
          {
            if (spDbManager->AddResourceArchive(spCurrentProject, url))
            {
              QStringList vsCompressedFiles;
              QDirIterator itCompressed(CPhysFsFileEngineHandler::c_sScheme + sRelativePath,
                                        QStringList() <<
                                          SResourceFormats::ImageFormats() <<
                                          SResourceFormats::VideoFormats() <<
                                          SResourceFormats::AudioFormats() <<
                                          SResourceFormats::FontFormats() <<
                                          SResourceFormats::OtherFormats() <<
                                          SResourceFormats::ScriptFormats() <<
                                          SResourceFormats::DatabaseFormats() <<
                                          SResourceFormats::LayoutFormats() <<
                                          SResourceFormats::SequenceFormat(),
                                        QDir::Files | QDir::NoDotAndDotDot,
                                        QDirIterator::Subdirectories);
              while (itCompressed.hasNext())
              {
                vsCompressedFiles << itCompressed.next();
              }
              auto mapGotten =
                  AddFilesToProjectResources(spCurrentProject, spSettings, wpDbManager,
                                             pParentForDialog, vsCompressedFiles);
              retMap.insert(mapGotten.begin(), mapGotten.end());
            }
          }

          if (!sResource.isEmpty())
          {
            if (tspResource spResource = spDbManager->FindResourceInProject(spCurrentProject, sResource))
            {
              // make a deep copy to store later
              retMap.insert({sResource,
                             std::make_shared<SResource>(*spResource)});
            }
          }
        }
      }
    }

    // handle action
    if (!vsNeedsToMove.isEmpty())
    {
      QMessageBox msgBox(pParentForDialog);
      msgBox.setText(QObject::tr("At least one file is not in the subfolder of project."));
      msgBox.setInformativeText(QObject::tr("Do you want to move or copy the file(s)?"));
      QPushButton* pMove = msgBox.addButton(QObject::tr("Move"), QMessageBox::AcceptRole);
      QPushButton* pCopy = msgBox.addButton(QObject::tr("Copy"), QMessageBox::ActionRole);
      QPushButton* pCancel = msgBox.addButton(QMessageBox::Cancel);
      msgBox.setDefaultButton(pCancel);
      msgBox.setModal(true);
      msgBox.setWindowFlag(Qt::FramelessWindowHint);

      msgBox.exec();
      if (nullptr == pParentForDialog)
      {
        return tspResourceMap();
      }

      QStringList filesToAdd;
      if (msgBox.clickedButton() == pMove)
      {
        // Move the Files
        const QString sDirToMoveTo = QFileDialog::getExistingDirectory(pParentForDialog,
            QObject::tr("Select Destination"), projectDir.absolutePath());
        for (QString sFileName : vsFiles)
        {
          QFileInfo info(sFileName);
          QFile file(info.absoluteFilePath());
          const QString sNewName = sDirToMoveTo + "/" + info.fileName();
          if (!file.rename(sNewName))
          {
            qWarning() << QString(QObject::tr("Renaming file '%1' failed.")).arg(sNewName);
          }
          else
          {
            if (sNewName.contains(projectDir.absolutePath()))
            {
              filesToAdd.push_back(sNewName);
            }
          }
        }
        auto mapGotten =
            AddFilesToProjectResources(spCurrentProject, spSettings, wpDbManager,
                                       pParentForDialog, filesToAdd);
        retMap.insert(mapGotten.begin(), mapGotten.end());
      }
      else if (msgBox.clickedButton() == pCopy)
      {
        // copy the Files
        const QString sDirToCopyTo = QFileDialog::getExistingDirectory(pParentForDialog,
            QObject::tr("Select Destination"), projectDir.absolutePath());
        for (QString sFileName : vsFiles)
        {
          QFileInfo info(sFileName);
          QFile file(info.absoluteFilePath());
          const QString sNewName = sDirToCopyTo + "/" + info.fileName();
          if (!file.copy(sNewName))
          {
            qWarning() << QString(QObject::tr("Copying file '%1' failed.")).arg(sNewName);
          }
          else
          {
            if (sNewName.contains(projectDir.absolutePath()))
            {
              filesToAdd.push_back(sNewName);
            }
          }
        }
        auto mapGotten =
            AddFilesToProjectResources(spCurrentProject, spSettings, wpDbManager,
                                       pParentForDialog, filesToAdd);
        retMap.insert(mapGotten.begin(), mapGotten.end());
      }
      else if (msgBox.clickedButton() == pCancel)
      {
        // nothing to do
      }
    }

    return retMap;
  }

//----------------------------------------------------------------------------------------
//
  tspResourceMap AddUrlsToProjectResources(std::shared_ptr<SProject> spCurrentProject,
                                           std::weak_ptr<CDatabaseManager> wpDbManager,
                                           const std::map<QUrl, QByteArray>& remoteFiles)
  {
    tspResourceMap retVal;
    for (const auto& it : remoteFiles)
    {
      const QUrl& url = it.first;
      const QByteArray& arr = it.second;

      QStringList imageFormatsList = SResourceFormats::ImageFormats();
      QStringList videoFormatsList = SResourceFormats::VideoFormats();

      qint32 iLastIndex = url.fileName().lastIndexOf('.');
      const QString sFileName = url.fileName();
      QString sFormat = "*" + sFileName.mid(iLastIndex, sFileName.size() - iLastIndex);
      auto spDbManager = wpDbManager.lock();
      if (nullptr != spDbManager)
      {
        if (imageFormatsList.contains(sFormat))
        {
          QPixmap mPixmap;
          mPixmap.loadFromData(arr);
          if (!mPixmap.isNull())
          {
            QString sName =
              spDbManager->AddResource(spCurrentProject, url, EResourceType::eImage);
            tspResource spResource = spDbManager->FindResourceInProject(spCurrentProject, sName);
            if (nullptr != spResource)
            {
              QWriteLocker locker(&spResource->m_rwLock);
              spResource->m_sSource = url;
            }
          }
        }
        else if (videoFormatsList.contains(sFormat))
        {
          // TODO: check video
          QString sName =
              spDbManager->AddResource(spCurrentProject, url, EResourceType::eMovie);
          tspResource spResource = spDbManager->FindResourceInProject(spCurrentProject, sName);
          if (nullptr != spResource)
          {
            QWriteLocker locker(&spResource->m_rwLock);
            spResource->m_sSource = url;
          }
        }
      }
    }
    return retVal;
  }

//----------------------------------------------------------------------------------------
//
  QString CommandTextForFiles(const std::map<QUrl, QByteArray>& files)
  {
    if (0 == files.size()) { return QString("Adding 0 resources."); }
    else if (1 == files.size())
    {
      return QString("Adding resource file %1.").arg(QFileInfo(files.begin()->first.toString()).fileName());
    }
    else { return QString("Adding %1 resources.").arg(files.size()); }
  }
  QString CommandTextForResources(const tspResourceMap& files)
  {
    if (0 == files.size()) { return QString("Adding 0 resources."); }
    else if (1 == files.size())
    {
      return QString("Adding resource file %1.").arg(files.begin()->first);
    }
    else { return QString("Adding %1 resources.").arg(files.size()); }
  }
}

//----------------------------------------------------------------------------------------
//
CCommandAddResource::CCommandAddResource(const tspProject& spProject,
                                         QPointer<QWidget> pParentForDialog,
                                         const std::map<QUrl, QByteArray>& vsFiles,
                                         QUndoCommand* pParent) :
  QUndoCommand(CommandTextForFiles(vsFiles), pParent),
  m_addedResources(),
  m_spCurrentProject(spProject),
  m_spSettings(CApplication::Instance()->Settings()),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pParentForDialog(pParentForDialog),
  m_vsFiles(vsFiles)
{

}
CCommandAddResource::~CCommandAddResource()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandAddResource::undo()
{
  if (!m_addedResources.empty())
  {
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      for (auto& resourceIt : m_addedResources)
      {
        spDbManager->RemoveResource(m_spCurrentProject, resourceIt.first);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandAddResource::redo()
{
  if (m_addedResources.empty())
  {
    QStringList vsLocalFiles;
    std::map<QUrl, QByteArray> remoteFiles;
    for (const auto& file : qAsConst(m_vsFiles))
    {
      if (IsLocalFile(file.first))
      {
        vsLocalFiles << file.first.toString(QUrl::PreferLocalFile);
      }
      else { remoteFiles.insert(file); }
    }

    // first insertion, remember resources
    m_addedResources = AddFilesToProjectResources(m_spCurrentProject, m_spSettings, m_wpDbManager,
                                                  m_pParentForDialog, vsLocalFiles);
    tspResourceMap remoteResources = AddUrlsToProjectResources(m_spCurrentProject,
                                                               m_wpDbManager,
                                                               remoteFiles);
    m_addedResources.insert(remoteResources.begin(), remoteResources.end());

    // clear byearrays to not use too much memory if not nesseccary
    for (auto& file : m_vsFiles)
    {
      file.second.clear();
    }
  }
  else
  {
    // re-add removed resources
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      for (auto& resourceIt : m_addedResources)
      {
        const QString sName = spDbManager->AddResource(m_spCurrentProject,
                                                       QUrl(resourceIt.second->m_sPath),
                                                       resourceIt.second->m_type,
                                                       resourceIt.first);
        tspResource spAdded = spDbManager->FindResourceInProject(m_spCurrentProject, sName);
        if (nullptr != spAdded)
        {
          // set source by hand
          spAdded->m_rwLock.lockForWrite();
          spAdded->m_sSource = resourceIt.second->m_sSource;
          spAdded->m_rwLock.unlock();
          // store a copy
          resourceIt.second.reset(new SResource(*spAdded));
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandAddResource::id() const
{
  return EEditorCommandId::eAddResource;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddResource::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandAddResource* pOtherCasted = dynamic_cast<const CCommandAddResource*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_addedResources.insert(pOtherCasted->m_addedResources.begin(),
                          pOtherCasted->m_addedResources.end());
  setText(CommandTextForResources(m_addedResources));
  return true;
}
