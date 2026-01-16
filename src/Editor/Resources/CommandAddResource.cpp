#include "CommandAddResource.h"
#include "Application.h"
#include "Editor/EditorCommandIds.h"
#include "Settings.h"
#include "Systems/DatabaseManager.h"
#include "Systems/PhysFs/PhysFsFileEngine.h"
#include "Systems/Database/Resource.h"

#include <QDebug>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>

namespace
{
  //--------------------------------------------------------------------------------------
  //
  SResourcePath NormaliseResourceFromPhysFs(const QString& sPath)
  {
    if (sPath.startsWith(CPhysFsFileEngineHandler::c_sScheme))
    {
      static QString sPfs = QString(CPhysFsFileEngineHandler::c_sScheme).replace(":/", ":");
      return SResourcePath(QString(sPath).replace(CPhysFsFileEngineHandler::c_sScheme + "/", sPfs));
    }
    return SResourcePath(sPath);
  }

  //--------------------------------------------------------------------------------------
  //
  tspResourceMap AddArchivedResources(std::shared_ptr<SProject> spCurrentProject,
                                      std::weak_ptr<CDatabaseManager> wpDbManager,
                                      const std::vector<SResourcePath>& vsFiles,
                                      const QString& sBundleName);

  //--------------------------------------------------------------------------------------
  //
  tspResourceMap AddFilesToProjectResources(std::shared_ptr<SProject> spCurrentProject,
                                            std::weak_ptr<CDatabaseManager> wpDbManager,
                                            QPointer<QWidget> pParentForDialog,
                                            const std::vector<SResourcePath>& vsFiles,
                                            const QString& sBundleName = QString())
  {
    if (nullptr == spCurrentProject) { return tspResourceMap(); }

    QString sProjName;
    {
      QReadLocker l(&spCurrentProject->m_rwLock);
      sProjName = spCurrentProject->m_sName;
    }

    // add file to respective category
    std::vector<SResourcePath> vsNeedsToMove;
    tspResourceMap retMap;
    const QDir projectDir = PhysicalProjectPath(spCurrentProject);
    for (const SResourcePath& sFileName : vsFiles)
    {
      QFileInfo info(SResource::PhysicalResourcePath(sFileName, spCurrentProject, sBundleName));
      if (!info.absoluteFilePath().contains(projectDir.absolutePath()) &&
          !static_cast<QString>(sFileName).startsWith(CPhysFsFileEngineHandler::c_sScheme) &&
          !static_cast<QString>(sFileName).startsWith("qrc:/"+sProjName))
      {
        vsNeedsToMove.push_back(sFileName);
      }
      else
      {
        const QString sEnding = "*." + sFileName.Suffix();
        auto spDbManager = wpDbManager.lock();
        if (nullptr != spDbManager)
        {
          QString sResource;
          if (SResourceFormats::ImageFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, sFileName, EResourceType::eImage, QString(), sBundleName);
          }
          else if (SResourceFormats::VideoFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, sFileName, EResourceType::eMovie, QString(), sBundleName);
          }
          else if (SResourceFormats::AudioFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, sFileName, EResourceType::eSound, QString(), sBundleName);
          }
          else if (SResourceFormats::OtherFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, sFileName, EResourceType::eOther, QString(), sBundleName);
          }
          else if (SResourceFormats::ScriptFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, sFileName, EResourceType::eScript, QString(), sBundleName);
          }
          else if (SResourceFormats::DatabaseFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, sFileName, EResourceType::eDatabase, QString(), sBundleName);
          }
          else if (SResourceFormats::FontFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, sFileName, EResourceType::eFont, QString(), sBundleName);
          }
          else if (SResourceFormats::LayoutFormats().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, sFileName, EResourceType::eLayout, QString(), sBundleName);
          }
          else if (SResourceFormats::SequenceFormat().contains(sEnding))
          {
            sResource = spDbManager->AddResource(spCurrentProject, sFileName, EResourceType::eSequence, QString(), sBundleName);
          }
          else if (SResourceFormats::ArchiveFormats().contains(sEnding))
          {
            QStringList vsOldArchivedResources;
            QDirIterator itCompressed(":/" + sProjName,
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
              vsOldArchivedResources.push_back(itCompressed.next());
            }

            if (spDbManager->AddResourceArchive(spCurrentProject, sFileName))
            {
              std::vector<SResourcePath> vsCompressedFiles;
              if (sFileName.Suffix() == joip_resource::c_sResourceBundleSuffix)
              {
                auto spBundle =
                    spDbManager->FindResourceBundleInProject(spCurrentProject, sFileName.CompleteBaseName());

                QDirIterator itCompressed(":/" + sProjName,
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
                  QString sPotentialResource = itCompressed.next();
                  if (!vsOldArchivedResources.contains(sPotentialResource))
                  {
                    vsCompressedFiles.push_back(QUrl("qrc" + sPotentialResource));
                  }
                }

                auto mapGotten = AddArchivedResources(spCurrentProject, spDbManager,
                                                      vsCompressedFiles,
                                                      sFileName.CompleteBaseName());
                retMap.insert(mapGotten.begin(), mapGotten.end());
              }
              else
              {
                QDirIterator itCompressed(SResource::ResourceToAbsolutePath(sFileName, spCurrentProject),
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
                  vsCompressedFiles.push_back(NormaliseResourceFromPhysFs(itCompressed.next()));
                }

                auto mapGotten =
                    AddFilesToProjectResources(spCurrentProject, wpDbManager,
                                               pParentForDialog, vsCompressedFiles);
                retMap.insert(mapGotten.begin(), mapGotten.end());
              }
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
    if (!vsNeedsToMove.empty())
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

      std::vector<SResourcePath> filesToAdd;
      if (msgBox.clickedButton() == pMove)
      {
        // Move the Files
        const QString sDirToMoveTo = QFileDialog::getExistingDirectory(pParentForDialog,
            QObject::tr("Select Destination"), projectDir.absolutePath());
        for (const SResourcePath& sFileName : vsFiles)
        {
          QFileInfo info(SResource::PhysicalResourcePath(sFileName, spCurrentProject));
          QFile file(info.absoluteFilePath());
          const QString sNewName = sDirToMoveTo + "/" + sFileName.FileName();
          if (!file.rename(sNewName))
          {
            qWarning() << QString(QObject::tr("Renaming file '%1' failed.")).arg(sNewName);
          }
          else
          {
            if (sNewName.contains(projectDir.absolutePath()))
            {
              filesToAdd.push_back(
                  joip_resource::CreatePathFromAbsolutePath(sNewName, spCurrentProject));
            }
          }
        }
        auto mapGotten =
            AddFilesToProjectResources(spCurrentProject, wpDbManager,
                                       pParentForDialog, filesToAdd);
        retMap.insert(mapGotten.begin(), mapGotten.end());
      }
      else if (msgBox.clickedButton() == pCopy)
      {
        // copy the Files
        const QString sDirToCopyTo = QFileDialog::getExistingDirectory(pParentForDialog,
            QObject::tr("Select Destination"), projectDir.absolutePath());
        for (const SResourcePath& sFileName : vsFiles)
        {
          QFileInfo info(SResource::PhysicalResourcePath(sFileName, spCurrentProject));
          QFile file(info.absoluteFilePath());
          const QString sNewName = sDirToCopyTo + "/" + sFileName.FileName();
          if (!file.copy(sNewName))
          {
            qWarning() << QString(QObject::tr("Copying file '%1' failed.")).arg(sNewName);
          }
          else
          {
            if (sNewName.contains(projectDir.absolutePath()))
            {
              filesToAdd.push_back(
                  joip_resource::CreatePathFromAbsolutePath(sNewName, spCurrentProject));
            }
          }
        }
        auto mapGotten =
            AddFilesToProjectResources(spCurrentProject, wpDbManager,
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

  //--------------------------------------------------------------------------------------
  //
  tspResourceMap AddArchivedResources(std::shared_ptr<SProject> spCurrentProject,
                                      std::weak_ptr<CDatabaseManager> wpDbManager,
                                      const std::vector<SResourcePath>& vsFiles,
                                      const QString& sBundleName)
  {
    auto mapGotten =
        AddFilesToProjectResources(spCurrentProject, wpDbManager, nullptr, vsFiles,
                                   sBundleName);
    return mapGotten;
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
    std::vector<SResourcePath> vsLocalFiles;
    std::map<QUrl, QByteArray> remoteFiles;
    for (const auto& file : qAsConst(m_vsFiles))
    {
      if (SResourcePath::IsLocalFileP(file.first))
      {
        vsLocalFiles.push_back(joip_resource::CreatePathFromAbsoluteUrl(file.first, m_spCurrentProject));
      }
      else { remoteFiles.insert(file); }
    }

    // first insertion, remember resources
    m_addedResources = AddFilesToProjectResources(m_spCurrentProject, m_wpDbManager,
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
          // set old data manually
          spAdded->m_rwLock.lockForWrite();
          spAdded->CopyFrom(*resourceIt.second);
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
