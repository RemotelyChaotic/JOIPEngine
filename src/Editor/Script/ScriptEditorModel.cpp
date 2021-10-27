#include "ScriptEditorModel.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"

#include <QPushButton>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>

namespace
{
  const char c_sIdProperty[] = "ID";
}

//----------------------------------------------------------------------------------------
//
CScriptEditorModel::CScriptEditorModel(QWidget* pParent) :
  QStandardItemModel(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_spProject(),
  m_pParentWidget(pParent),
  m_cachedScriptsMap(),
  m_bReloadFileWithoutQuestion(false)
{
  auto spDbManager = m_wpDbManager.lock();
  connect(spDbManager.get(), &CDatabaseManager::SignalResourceAdded,
          this, &CScriptEditorModel::SlotResourceAdded, Qt::QueuedConnection);
  connect(spDbManager.get(), &CDatabaseManager::SignalResourceRemoved,
          this, &CScriptEditorModel::SlotResourceRemoved, Qt::QueuedConnection);
  connect(spDbManager.get(), &CDatabaseManager::SignalResourceRenamed,
          this, &CScriptEditorModel::SlotResourceRenamed, Qt::QueuedConnection);

  connect(spDbManager.get(), &CDatabaseManager::SignalSceneRemoved,
          this, &CScriptEditorModel::SlotSceneRemoved, Qt::QueuedConnection);
  connect(spDbManager.get(), &CDatabaseManager::SignalSceneRenamed,
          this, &CScriptEditorModel::SlotSceneRenamed, Qt::QueuedConnection);
}

CScriptEditorModel::~CScriptEditorModel()
{
  DeInitializeModel();
}

//----------------------------------------------------------------------------------------
//
SCachedMapItem* CScriptEditorModel::CachedScript(const QString& sName)
{
  auto it = m_cachedScriptsMap.find(sName);
  if (m_cachedScriptsMap.end() != it)
  {
    if (!it->second.m_bInitialized)
    {
      LoadScriptFile(it->first);
    }
    return &it->second;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
QString CScriptEditorModel::CachedScriptName(qint32 iIndex)
{
  if (static_cast<qint32>(m_cachedScriptsMap.size()) > iIndex && 0 <= iIndex)
  {
    auto it = m_cachedScriptsMap.begin();
    std::advance(it, iIndex);
    return it->first;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::InitializeModel(tspProject spProject)
{
  m_spProject = spProject;

  beginResetModel();

  // need to cache it, so the actual model can be updated between beginInsertRows and endInsertRows
  // otherwise beginInsertRows checks the wrong number of existing elements
  if (nullptr != m_spProject)
  {
    QReadLocker projectLocker(&m_spProject->m_rwLock);
    for (auto it = m_spProject->m_spResourcesMap.begin(); m_spProject->m_spResourcesMap.end() != it; ++it)
    {
      AddResourceTo(it->second, m_cachedScriptsMap);
    }
  }

  endResetModel();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::DeInitializeModel()
{
  beginResetModel();
  m_cachedScriptsMap.clear();
  endResetModel();
  m_spProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::SerializeProject()
{
  if (nullptr == m_spProject) { return; }

  // iterate through and save content
  for (auto it = m_cachedScriptsMap.begin(); m_cachedScriptsMap.end() != it; ++it)
  {
    if (it->second.m_bInitialized && it->second.m_bChanged)
    {
      SetSceneScriptModifiedFlag(it->first, false);

      auto spDbManager = m_wpDbManager.lock();
      if (nullptr != spDbManager)
      {
        QReadLocker projLocker(&m_spProject->m_rwLock);
        const QString sProjectName = m_spProject->m_sFolderName;
        projLocker.unlock();

        auto spResource = spDbManager->FindResourceInProject(m_spProject, it->first);
        if (nullptr != spResource)
        {
          const QString sFilePath = ResourceUrlToAbsolutePath(spResource);
          QReadLocker lockerResource(&spResource->m_rwLock);

          QFile file(sFilePath);
          if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
          {
            it->second.m_bIgnoreNextModification = true;
            file.write(it->second.m_data);
          }
          else
          {
            qWarning() << "Registered script resource could not be opened.";
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CScriptEditorModel::ScriptIndex(const QString& sName)
{
  auto it = m_cachedScriptsMap.find(sName);
  if (m_cachedScriptsMap.end() != it)
  {
    return static_cast<qint32>(std::distance(m_cachedScriptsMap.begin(), it));
  }
  return -1;
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::SetReloadFileWithoutQuestion(bool bReload)
{
  m_bReloadFileWithoutQuestion = bReload;
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::SetSceneScriptModifiedFlag(const QString& sName, bool bModified)
{
  auto it = m_cachedScriptsMap.find(sName);
  if (m_cachedScriptsMap.end() != it)
  {
    QModelIndex index =
        createIndex(static_cast<qint32>(std::distance(m_cachedScriptsMap.begin(), it)),
                    0, nullptr);
    it->second.m_bChanged = bModified;
    if (bModified) { emit SignalProjectEdited(); }
    emit dataChanged(index, index);
  }
}

//----------------------------------------------------------------------------------------
//
QModelIndex CScriptEditorModel::index(int iRow, int iCol, const QModelIndex& parent) const
{
  if (iCol == 0 &&
      0 <= iRow &&
      rowCount(parent) > iRow)
  {
    return createIndex(iRow, iCol, nullptr);
  }
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
int CScriptEditorModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return static_cast<qint32>(m_cachedScriptsMap.size());
}

//----------------------------------------------------------------------------------------
//
int CScriptEditorModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return 1;
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEditorModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid()) { return QVariant(); }
  qint32 iRow = index.row();
  if (m_cachedScriptsMap.size() > static_cast<size_t>(iRow))
  {
    auto it = m_cachedScriptsMap.begin();
    std::advance(it, iRow);

    if (Qt::DisplayRole == iRole)
    {
      QString sSceneName;
      if (nullptr != it->second.m_spScene)
      {
        QReadLocker locker(&it->second.m_spScene->m_rwLock);
        sSceneName = it->second.m_spScene->m_sName;
      }
      return it->first +
          (!sSceneName.isEmpty() ? QString(tr(" (Scene: %1) ")).arg(sSceneName) : "") +
          (it->second.m_bChanged ? "*" : "");
    }
    else
    {
      return QVariant();
    }
  }
  else
  {
    return QVariant();
  }
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CScriptEditorModel::flags(const QModelIndex& index) const
{
  return QStandardItemModel::flags(index);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::SlotFileChanged(const QString& sPath)
{
  Q_UNUSED(sPath);
  QFileSystemWatcher* pWatcher = qobject_cast<QFileSystemWatcher*>(sender());
  if (nullptr != pWatcher)
  {
    const QString sId = pWatcher->property(c_sIdProperty).toString();
    auto it = m_cachedScriptsMap.find(sId);
    if (m_cachedScriptsMap.end() != it && it->second.m_bInitialized)
    {
      if (it->second.m_bIgnoreNextModification)
      {
        it->second.m_bIgnoreNextModification = false;
      }
      else
      {
        if (!it->second.m_bAllreadyAsked)
        {
          it->second.m_bIgnoreNextModification = true;
          it->second.m_bAllreadyAsked = true;
          if (!m_bReloadFileWithoutQuestion)
          {
            QMessageBox msgBox(m_pParentWidget);
            msgBox.setText("The document has been modified on the disc.");
            msgBox.setInformativeText("Do you want to reload the file or keep the local file?");
            QPushButton* pReloadButton = msgBox.addButton(tr("Reload"), QMessageBox::AcceptRole);
            QPushButton* pKeepButton = msgBox.addButton(tr("Keep"), QMessageBox::RejectRole);
            msgBox.setDefaultButton(pKeepButton);
            msgBox.exec();

            SetSceneScriptModifiedFlag(it->first, msgBox.clickedButton() == pKeepButton);

            if (msgBox.clickedButton() == pReloadButton)
            {
              LoadScriptFile(sId);
              emit SignalFileChangedExternally(sId);
            }
          }
          else
          {
            LoadScriptFile(sId);
          }
          it->second.m_bAllreadyAsked = false;
          it->second.m_bIgnoreNextModification = false;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::SlotResourceAdded(qint32 iProjId, const QString& sName)
{
  if (nullptr == m_spProject) { return; }
  m_spProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spProject->m_iId;
  QString sProjectFolder = m_spProject->m_sFolderName;
  m_spProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    auto spResource = spDbManager->FindResourceInProject(m_spProject, sName);
    if (nullptr != spResource)
    {
      QReadLocker locker(&spResource->m_rwLock);
      if (spResource->m_type._to_integral() != EResourceType::eScript)
      { return; }
      locker.unlock();

      // find new index
      std::vector<QString> vsNamesInOrder;
      vsNamesInOrder.reserve(m_cachedScriptsMap.size());
      for (const auto& item : m_cachedScriptsMap)
      {
        vsNamesInOrder.push_back(item.first);
      }
      qint32 iTargetIndex = 0;
      auto compOp = m_cachedScriptsMap.key_comp();
      for (iTargetIndex = 0; iTargetIndex < static_cast<qint32>(vsNamesInOrder.size()); iTargetIndex++)
      {
        if (!compOp(vsNamesInOrder[static_cast<size_t>(iTargetIndex)], sName))
        {
          break;
        }
      }

      beginInsertRows(QModelIndex(), iTargetIndex, iTargetIndex);
      AddResourceTo(spResource, m_cachedScriptsMap);
      endInsertRows();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::SlotResourceRemoved(qint32 iProjId, const QString& sName)
{
  if (nullptr == m_spProject) { return; }
  m_spProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spProject->m_iId;
  m_spProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    auto it = m_cachedScriptsMap.find(sName);
    if (m_cachedScriptsMap.end() != it)
    {
      qint32 iTargetIndex = static_cast<qint32>(std::distance(m_cachedScriptsMap.begin(), it));
      beginRemoveRows(QModelIndex(), iTargetIndex, iTargetIndex);
      m_cachedScriptsMap.erase(it);
      endRemoveRows();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::SlotResourceRenamed(qint32 iProjId, const QString& sOldName,
                                             const QString& sName)
{
  if (nullptr == m_spProject) { return; }
  m_spProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spProject->m_iId;
  m_spProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    SlotResourceRemoved(iProjId, sOldName);
    SlotResourceAdded(iProjId, sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::SlotSceneRenamed(qint32 iProjId, qint32 iId)
{
  if (nullptr == m_spProject) { return; }
  m_spProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spProject->m_iId;
  m_spProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    for (auto it = m_cachedScriptsMap.begin(); m_cachedScriptsMap.end() != it; ++it)
    {
      auto spScene = it->second.m_spScene;
      if (nullptr != spScene)
      {
        QReadLocker locker(&spScene->m_rwLock);
        qint32 iIdSaved = spScene->m_iId;
        locker.unlock();

        if (iId == iIdSaved)
        {
          QModelIndex index =
              createIndex(static_cast<qint32>(std::distance(m_cachedScriptsMap.begin(), it)),
                          0, nullptr);
          emit dataChanged(index, index);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::SlotSceneRemoved(qint32 iProjId, qint32 iId)
{
  if (nullptr == m_spProject) { return; }
  m_spProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spProject->m_iId;
  m_spProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    for (auto it = m_cachedScriptsMap.begin(); m_cachedScriptsMap.end() != it; ++it)
    {
      auto spScene = it->second.m_spScene;
      if (nullptr != spScene)
      {
        QReadLocker locker(&spScene->m_rwLock);
        qint32 iIdSaved = spScene->m_iId;
        locker.unlock();

        if (iId == iIdSaved)
        {
          it->second.m_spScene = nullptr;
          QModelIndex index =
              createIndex(static_cast<qint32>(std::distance(m_cachedScriptsMap.begin(), it)),
                          0, nullptr);
          emit dataChanged(index, index);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::AddResourceTo(tspResource spResource, std::map<QString, SCachedMapItem>& mpToAddTo)
{
  if (nullptr == m_spProject) { return; }

  QString sPath = ResourceUrlToAbsolutePath(spResource);
  QReadLocker resourceLocker(&spResource->m_rwLock);
  const QString sResourceName = spResource->m_sName;
  if (spResource->m_type._to_integral() != EResourceType::eScript)
  {
    return;
  }

  QFileInfo scriptFileInfo(sPath);
  if (scriptFileInfo.exists())
  {
    auto itMap = mpToAddTo.find(sResourceName);
    if (mpToAddTo.end() == itMap)
    {
      mpToAddTo.insert({sResourceName, SCachedMapItem()});
      auto& script = mpToAddTo[sResourceName];
      script.m_sId = sResourceName;
      if (!script.m_spWatcher->addPath(sPath))
      {
        qWarning() << QString(tr("Could not add %1 to the watched paths."))
                      .arg(sPath);
      }
      script.m_spWatcher->setProperty(c_sIdProperty, sResourceName);
      connect(script.m_spWatcher.get(), &QFileSystemWatcher::fileChanged,
              this, &CScriptEditorModel::SlotFileChanged, Qt::UniqueConnection);

      for (const auto& spScene : m_spProject->m_vspScenes)
      {
        QReadLocker sceneLocker(&spScene->m_rwLock);
        if (spScene->m_sScript == sResourceName)
        {
          script.m_spScene = spScene;
          break;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorModel::LoadScriptFile(const QString& sName)
{
  if (nullptr == m_spProject) { return; }

  auto it = m_cachedScriptsMap.find(sName);
  if (m_cachedScriptsMap.end() != it)
  {
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      QReadLocker projLocker(&m_spProject->m_rwLock);
      const QString sProjectName = m_spProject->m_sFolderName;
      projLocker.unlock();

      auto spResource = spDbManager->FindResourceInProject(m_spProject, sName);
      if (nullptr != spResource)
      {
        const QString sFilePath = ResourceUrlToAbsolutePath(spResource);
        QReadLocker lockerResource(&spResource->m_rwLock);

        QFile file(sFilePath);
        if (file.open(QIODevice::ReadOnly))
        {
          it->second.m_data = file.readAll();
          it->second.m_bInitialized = true;
        }
        else
        {
          qWarning() << "Registered script resource could not be opened.";
        }
      }
    }
  }
}
