#include "CommandAddNewSequence.h"
#include "Application.h"
#include "Settings.h"

#include "Editor/EditorCommandIds.h"

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
  void InitScript(QIODevice& file, const QString& sType)
  {
    auto itDefinition = SScriptDefinitionData::DefinitionMap().find(sType);
    if (SScriptDefinitionData::DefinitionMap().end() != itDefinition)
    {
      file.write(itDefinition->second.sInitText.toUtf8());
    }
  }

  //--------------------------------------------------------------------------------------
  //
  tspResource AddNewSequenceFile(tspProject spProject,
                                 std::weak_ptr<CDatabaseManager> m_wpDbManager,
                                 QPointer<QWidget> pParentForDialog)
  {
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spProject && nullptr != spDbManager)
    {
      QStringList formats = SResourceFormats::SequenceFormat();

      const QString sProjectPath = PhysicalProjectPath(spProject);
      QDir projectDir(sProjectPath);
      QFileDialog* dlg = new QFileDialog(pParentForDialog,
                                         QObject::tr("Create Sequence File"));
      dlg->setViewMode(QFileDialog::Detail);
      dlg->setFileMode(QFileDialog::AnyFile);
      dlg->setAcceptMode(QFileDialog::AcceptSave);
      dlg->setOptions(QFileDialog::DontUseCustomDirectoryIcons);
      dlg->setDirectoryUrl(projectDir.absolutePath());
      dlg->setFilter(QDir::AllDirs);
      dlg->setNameFilter(QString("Files (%1)").arg(formats.join(" ")));
      dlg->setDefaultSuffix(formats.first());

      if (dlg->exec())
      {
        if (nullptr == pParentForDialog) { return nullptr; }
        QList<QUrl> urls = dlg->selectedUrls();
        delete dlg;

        QUrl url = 1 == urls.size() ? urls[0] : QUrl();
        if (url.isValid())
        {
          QFileInfo info(url.toLocalFile());
          if (!info.absoluteFilePath().contains(projectDir.absolutePath()))
          {
            qWarning() << "File is not in subfolder of Project.";
          }
          else
          {
            QString sRelativePath = projectDir.relativeFilePath(info.absoluteFilePath());
            QUrl sUrlToSave = ResourceUrlFromLocalFile(sRelativePath);
            QFile scriptFile(info.absoluteFilePath());
            if (scriptFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
              InitScript(scriptFile, info.suffix());
              QString sResource = spDbManager->AddResource(spProject, sUrlToSave,
                                                           EResourceType::eSequence,
                                                           QString());
              return spDbManager->FindResourceInProject(spProject, sResource);
            }
            else
            {
              qWarning() << "Could not write script file.";
            }
          }
        }
      }
      else
      {
        if (nullptr == pParentForDialog) { return nullptr; }
        delete dlg;
      }
    }
    return nullptr;
  }
}

//----------------------------------------------------------------------------------------
//
CCommandAddNewSequence::CCommandAddNewSequence(const tspProject& spProject,
                                               QPointer<QWidget> pParentForDialog,
                                               QUndoCommand* pParent) :
  QUndoCommand("Added sequence file", pParent),
  m_addedResource(),
  m_spCurrentProject(spProject),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pParentForDialog(pParentForDialog)
{
}
CCommandAddNewSequence::~CCommandAddNewSequence() = default;

//----------------------------------------------------------------------------------------
//
void CCommandAddNewSequence::undo()
{
  if (nullptr != m_addedResource)
  {
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      QReadLocker locker(&m_addedResource->m_rwLock);
      const QString sResource = m_addedResource->m_sName;
      locker.unlock();
      spDbManager->RemoveResource(m_spCurrentProject, sResource);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandAddNewSequence::redo()
{
  if (nullptr == m_addedResource)
  {
    tspResource spResource =
        AddNewSequenceFile(m_spCurrentProject, m_wpDbManager, m_pParentForDialog);
    m_addedResource = spResource;
  }
  else
  {
    // re-add removed resource
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      const QString sName = spDbManager->AddResource(m_spCurrentProject,
                                                     QUrl(m_addedResource->m_sPath),
                                                     m_addedResource->m_type,
                                                     m_addedResource->m_sName);
      tspResource spAdded = spDbManager->FindResourceInProject(m_spCurrentProject, sName);
      if (nullptr != spAdded)
      {
        // set source by hand
        spAdded->m_rwLock.lockForWrite();
        spAdded->m_sSource = m_addedResource->m_sSource;
        spAdded->m_rwLock.unlock();
        // store a copy
        m_addedResource.reset(new SResource(*spAdded));
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandAddNewSequence::id() const
{
  return EEditorCommandId::eAddSequence;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddNewSequence::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandAddNewSequence* pOtherCasted = dynamic_cast<const CCommandAddNewSequence*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  return false;
}
