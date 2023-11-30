#include "SceneNodeModel.h"
#include "Application.h"
#include "CommandNodeEdited.h"
#include "SceneNodeModelWidget.h"
#include "SceneTranstitionData.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"

namespace {
  const qint32 c_iInPorts = 1;
  const qint32 c_iOutPorts = 1;
}

CSceneNodeModel::CSceneNodeModel() :
  CEditorNodeModelBase(),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_spOutData(std::make_shared<CSceneTranstitionData>()),
  m_spProject(nullptr),
  m_spScene(nullptr),
  m_bOutConnected(false),
  m_modelValidationState(NodeValidationState::Warning),
  m_modelValidationError(QString(tr("Missing or incorrect inputs or output")))
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    connect(spDbManager.get(), &CDatabaseManager::SignalSceneDataChanged,
            this, &CSceneNodeModel::SlotSceneDataChanged);
    connect(spDbManager.get(), &CDatabaseManager::SignalSceneRenamed,
            this, &CSceneNodeModel::SlotSceneRenamed);
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceAdded,
            this, &CSceneNodeModel::SlotResourceAdded);
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceRenamed,
            this, &CSceneNodeModel::SlotResourceRenamed);
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceRemoved,
            this, &CSceneNodeModel::SlotResourceRemoved);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SetProjectId(qint32 iId)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spProject = spDbManager->FindProject(iId);
    if (m_sSceneName.isNull() || m_sSceneName.isEmpty())
    {
      qint32 iNewId = spDbManager->AddScene(m_spProject);
      m_spScene = spDbManager->FindScene(m_spProject, iNewId);
      if (nullptr != m_spScene)
      {
        m_spScene->m_rwLock.lockForRead();
        m_sSceneName = m_spScene->m_sName;
        m_sOldSceneName = m_sSceneName;
        m_spScene->m_rwLock.unlock();
      }
    }
    else
    {
      m_spScene = spDbManager->FindScene(m_spProject, m_sSceneName);
    }

    // compatibility with old versions that didn't save the script
    if (m_sScript.isEmpty())
    {
      QReadLocker locker(&m_spScene->m_rwLock);
      m_sScript = m_spScene->m_sScript;
    }

    ProjectSetImpl();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CSceneNodeModel::ProjectId()
{
  if (nullptr != m_spProject)
  {
    QReadLocker locker(&m_spProject->m_rwLock);
    return m_spProject->m_iId;
  }
  return -1;
}

//----------------------------------------------------------------------------------------
//
qint32 CSceneNodeModel::SceneId()
{
  if (nullptr != m_spScene)
  {
    QReadLocker locker(&m_spScene->m_rwLock);
    return m_spScene->m_iId;
  }
  return -1;
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SetSceneName(const QString& sScene)
{
  SlotNameChanged(sScene);
}

//----------------------------------------------------------------------------------------
//
QString CSceneNodeModel::caption() const
{
  return staticCaption();
}

//----------------------------------------------------------------------------------------
//
QString CSceneNodeModel::name() const
{
  return staticCaption();
}

//----------------------------------------------------------------------------------------
//
QJsonObject CSceneNodeModel::save() const
{
  QJsonObject modelJson = NodeDataModel::save();
  modelJson["sName"] = m_sSceneName;
  modelJson["sScript"] = m_sScript;
  modelJson["sLayout"] = m_sLayout;
  return modelJson;
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::restore(QJsonObject const& p)
{
  QJsonValue v = p["sName"];
  if (!v.isUndefined())
  {
    m_sSceneName = v.toString();
    m_sOldSceneName = m_sSceneName;
  }
  v = p["sScript"];
  if (!v.isUndefined())
  {
    m_sScript = v.toString();
  }
  v = p["sLayout"];
  if (!v.isUndefined())
  {
    m_sLayout = v.toString();
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::UndoRestore(QJsonObject const& p)
{
  m_bIsInUndoOperation = true;
  QJsonValue v = p["sName"];
  if (!v.isUndefined())
  {
    SlotNameChanged(v.toString());
    m_sOldSceneName = m_sSceneName;
  }
  v = p["sScript"];
  if (!v.isUndefined())
  {
    SlotScriptChanged(v.toString());
  }
  v = p["sLayout"];
  if (!v.isUndefined())
  {
    SlotLayoutChanged(v.toString());
  }
  m_bIsInUndoOperation = false;
}

//----------------------------------------------------------------------------------------
//
unsigned int CSceneNodeModel::nPorts(PortType portType) const
{
  unsigned int result;

  if (portType == PortType::In)
  {
    result = c_iInPorts;
  }
  else
  {
    result = c_iOutPorts;
  }

  return result;
}

//----------------------------------------------------------------------------------------
//
NodeDataType CSceneNodeModel::dataType(PortType portType, PortIndex portIndex) const
{
  Q_UNUSED(portType)
  Q_UNUSED(portIndex)
  return CSceneTranstitionData().type();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<NodeData> CSceneNodeModel::outData(PortIndex port)
{
  Q_UNUSED(port)
  return std::static_pointer_cast<NodeData>(m_spOutData);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::setInData(std::shared_ptr<NodeData> data, PortIndex portIndex)
{
  auto newData =
    std::dynamic_pointer_cast<CSceneTranstitionData>(data);

  if (portIndex == 0 && nullptr != newData)
  {
    PortIndex const outPortIndex = 0;

    m_wpInData = newData;

    if (m_bOutConnected)
    {
      m_modelValidationState = NodeValidationState::Valid;
      m_modelValidationError = QString();
    }
    else
    {
      m_modelValidationState = NodeValidationState::Warning;
      m_modelValidationError = QString(tr("Missing or incorrect inputs or output"));
    }

    emit dataUpdated(outPortIndex);
  }
}

//----------------------------------------------------------------------------------------
//
QWidget* CSceneNodeModel::embeddedWidget()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
NodeValidationState CSceneNodeModel::validationState() const
{
  return m_modelValidationState;
}

//----------------------------------------------------------------------------------------
//
QString CSceneNodeModel::validationMessage() const
{
  return m_modelValidationError;
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::outputConnectionCreated(QtNodes::Connection const& c)
{
  Q_UNUSED(c)
  m_bOutConnected = true;
  if (m_wpInData.expired())
  {
    m_modelValidationState = NodeValidationState::Warning;
    m_modelValidationError = QString(tr("Missing or incorrect inputs or output"));
  }
  else
  {
    m_modelValidationState = NodeValidationState::Valid;
    m_modelValidationError = QString();
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::outputConnectionDeleted(QtNodes::Connection const& c)
{
  Q_UNUSED(c)
  m_bOutConnected = false;
  m_modelValidationState = NodeValidationState::Warning;
  m_modelValidationError = QString(tr("Missing or incorrect inputs or output"));
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SlotNameChanged(const QString& sName)
{
  QJsonObject oldState = save();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (nullptr != m_spScene)
    {
      m_spScene->m_rwLock.lockForRead();
      qint32 iId = m_spScene->m_iId;
      m_spScene->m_rwLock.unlock();

      spDbManager->RenameScene(m_spProject, iId, sName);

      m_spScene->m_rwLock.lockForRead();
      QString sSceneNameAfterChange = m_spScene->m_sName;
      m_spScene->m_rwLock.unlock();

      SlotNameChangedImpl(sSceneNameAfterChange);
      m_sSceneName = sSceneNameAfterChange;

      if (nullptr != UndoStack() && !m_bIsInUndoOperation)
      {
        QJsonObject newState = save();
        UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SlotLayoutChanged(const QString& sName)
{
  QJsonObject oldState = save();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (nullptr != m_spScene)
    {
      qint32 iProjId = -1;
      m_spProject->m_rwLock.lockForRead();
      iProjId = m_spProject->m_iId;
      m_spProject->m_rwLock.unlock();

      bool bChanged = false;
      qint32 iSceneId = -1;
      m_spScene->m_rwLock.lockForRead();
      bChanged = m_spScene->m_sSceneLayout != sName;
      m_spScene->m_sSceneLayout = sName;
      iSceneId = m_spScene->m_iId;
      m_spScene->m_rwLock.unlock();

      if (bChanged)
      {
        SlotLayoutChangedImpl(sName);
        emit spDbManager->SignalSceneDataChanged(iProjId, iSceneId);
      }

      m_sLayout = sName;

      if (nullptr != UndoStack() && !m_bIsInUndoOperation)
      {
        QJsonObject newState = save();
        UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SlotScriptChanged(const QString& sName)
{
  QJsonObject oldState = save();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (nullptr != m_spScene)
    {
      qint32 iProjId = -1;
      m_spProject->m_rwLock.lockForRead();
      iProjId = m_spProject->m_iId;
      m_spProject->m_rwLock.unlock();

      bool bChanged = false;
      qint32 iSceneId = -1;
      m_spScene->m_rwLock.lockForRead();
      bChanged = m_spScene->m_sScript != sName;
      m_spScene->m_sScript = sName;
      iSceneId = m_spScene->m_iId;
      m_spScene->m_rwLock.unlock();

      if (bChanged)
      {
        SlotScriptChangedImpl(sName);
        emit spDbManager->SignalSceneDataChanged(iProjId, iSceneId);
      }

      m_sScript = sName;

      if (nullptr != UndoStack() && !m_bIsInUndoOperation)
      {
        QJsonObject newState = save();
        UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SlotSceneDataChanged(qint32 iProjId, qint32 iSceneId)
{
  m_spScene->m_rwLock.lockForRead();
  qint32 iId = m_spScene->m_iId;
  const QString sScript = m_spScene->m_sScript;
  const QString sLayout = m_spScene->m_sSceneLayout;
  m_spScene->m_rwLock.unlock();

  if (iProjId == ProjectId() && iSceneId == iId)
  {
    SlotLayoutChanged(sLayout);
    SlotScriptChanged(sScript);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SlotSceneRenamed(qint32 iProjId, qint32 iSceneId)
{
  m_spScene->m_rwLock.lockForRead();
  qint32 iId = m_spScene->m_iId;
  const QString sNewName = m_spScene->m_sName;
  m_spScene->m_rwLock.unlock();

  if (iProjId == ProjectId() && iSceneId == iId)
  {
    if (m_sOldSceneName != sNewName)
    {
      m_sOldSceneName = sNewName;
      SlotSceneRenamedImpl(sNewName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SlotResourceAdded(qint32 iProjId, const QString& sName)
{
  auto spDbManager = m_wpDbManager.lock();
  if (ProjectId() == iProjId && nullptr != m_spScene && nullptr != spDbManager)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sName);
    if (nullptr != spResource)
    {
      QReadLocker locker(&spResource->m_rwLock);
      if (EResourceType::eScript == spResource->m_type._to_integral() ||
          EResourceType::eLayout == spResource->m_type._to_integral())
      {
        SlotResourceAddedImpl(sName, spResource->m_type);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SlotResourceRenamed(qint32 iProjId,
                                          const QString& sOldName, const QString& sName)
{
  auto spDbManager = m_wpDbManager.lock();
  if (ProjectId() == iProjId && nullptr != m_spScene && nullptr != spDbManager)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sName);
    if (nullptr != spResource)
    {
      QReadLocker locker(&spResource->m_rwLock);
      if (EResourceType::eScript == spResource->m_type._to_integral() ||
          EResourceType::eLayout == spResource->m_type._to_integral())
      {
        SlotResourceRenamedImpl(sOldName, sName, spResource->m_type);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SlotResourceRemoved(qint32 iProjId, const QString& sName)
{
  auto spDbManager = m_wpDbManager.lock();
  if (ProjectId() == iProjId && nullptr != m_spScene && nullptr != spDbManager)
  {
    // the resource is already removed, just remove from both lists, since names
    // are unique
    SlotResourceRemovedImpl(sName, EResourceType::eScript);
    SlotResourceRemovedImpl(sName, EResourceType::eLayout);
  }
}

//----------------------------------------------------------------------------------------
//
CSceneNodeModelWithWidget::CSceneNodeModelWithWidget() :
  CSceneNodeModel(),
  m_pWidget(new CSceneNodeModelWidget())
{
  connect(m_pWidget, &CSceneNodeModelWidget::SignalNameChanged,
          this, &CSceneNodeModelWithWidget::SlotNameChanged);
  connect(m_pWidget, &CSceneNodeModelWidget::SignalLayoutChanged,
          this, &CSceneNodeModelWithWidget::SlotLayoutChanged);
  connect(m_pWidget, &CSceneNodeModelWidget::SignalScriptChanged,
          this, &CSceneNodeModelWithWidget::SlotScriptChanged);
  connect(m_pWidget, &CSceneNodeModelWidget::SignalAddScriptFileClicked,
          this, &CSceneNodeModelWithWidget::SignalAddScriptFileRequested);
  connect(m_pWidget, &CSceneNodeModelWidget::SignalAddLayoutFileClicked,
          this, &CSceneNodeModelWithWidget::SignalAddLayoutFileRequested);
}
CSceneNodeModelWithWidget::~CSceneNodeModelWithWidget()
{

}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::SetProjectId(qint32 iId)
{
  CSceneNodeModel::SetProjectId(iId);
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetName(m_sSceneName);
    if (nullptr != m_spScene)
    {
      QReadLocker locker(&m_spScene->m_rwLock);
      if (!m_spScene->m_sScript.isEmpty())
      {
        m_pWidget->SetScript(m_spScene->m_sScript);
      }
      if (!m_spScene->m_sSceneLayout.isEmpty())
      {
        m_pWidget->SetLayout(m_spScene->m_sSceneLayout);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::SetSceneName(const QString& sScene)
{
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetName(sScene);
  }
  CSceneNodeModel::SetSceneName(sScene);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::restore(QJsonObject const& p)
{
  CSceneNodeModel::restore(p);
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetName(m_sSceneName);
    m_pWidget->SetScript(m_sScript);
    m_pWidget->SetLayout(m_sLayout);
  }
}

//----------------------------------------------------------------------------------------
//
QWidget* CSceneNodeModelWithWidget::embeddedWidget()
{
  return m_pWidget;
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::OnUndoStackSet()
{
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetUndoStack(m_pUndoStack);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::ProjectSetImpl()
{
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetProject(m_spProject);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::SlotNameChangedImpl(const QString& sName)
{
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetName(sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::SlotLayoutChangedImpl(const QString& sName)
{
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetLayout(sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::SlotScriptChangedImpl(const QString& sName)
{
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetScript(sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::SlotSceneRenamedImpl(const QString& sName)
{
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetName(sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::SlotResourceAddedImpl(const QString& sName, EResourceType type)
{
  if (nullptr != m_pWidget)
  {
    if (EResourceType::eScript == type._to_integral())
    {
      m_pWidget->OnScriptAdded(sName);
    }
    else if (EResourceType::eLayout == type._to_integral())
    {
      m_pWidget->OnLayoutAdded(sName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::SlotResourceRenamedImpl(const QString& sOldName,
                                                        const QString& sName,
                                                        EResourceType type)
{
  if (nullptr != m_pWidget)
  {
    if (EResourceType::eScript == type._to_integral())
    {
      m_pWidget->OnScriptRenamed(sOldName, sName);
    }
    else if (EResourceType::eLayout == type._to_integral())
    {
      m_pWidget->OnLayoutRenamed(sOldName, sName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWithWidget::SlotResourceRemovedImpl(const QString& sName,
                                                        EResourceType type)
{
  if (nullptr != m_pWidget)
  {
    if (EResourceType::eScript == type._to_integral())
    {
      m_pWidget->OnScriptRemoved(sName);
    }
    else if (EResourceType::eLayout == type._to_integral())
    {
      m_pWidget->OnLayoutRemoved(sName);
    }
  }
}
