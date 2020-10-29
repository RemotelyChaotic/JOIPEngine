#include "SceneNodeModel.h"
#include "Application.h"
#include "SceneNodeModelWidget.h"
#include "SceneTranstitionData.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"

namespace {
  const qint32 c_iInPorts = 1;
  const qint32 c_iOutPorts = 1;
}

CSceneNodeModel::CSceneNodeModel() :
  NodeDataModel(),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_spOutData(std::make_shared<CSceneTranstitionData>()),
  m_spProject(nullptr),
  m_spScene(nullptr),
  m_pWidget(new CSceneNodeModelWidget()),
  m_bOutConnected(false),
  m_modelValidationState(NodeValidationState::Warning),
  m_modelValidationError(QString(tr("Missing or incorrect inputs or output"))),
  m_sSceneName(),
  m_sOldSceneName()
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    connect(spDbManager.get(), &CDatabaseManager::SignalSceneRenamed,
            this, &CSceneNodeModel::SlotSceneRenamed);
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceAdded,
            this, &CSceneNodeModel::SlotResourceAdded);
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceRemoved,
            this, &CSceneNodeModel::SlotResourceRemoved);
  }

  connect(m_pWidget, &CSceneNodeModelWidget::SignalNameChanged,
          this, &CSceneNodeModel::SlotNameChanged);
  connect(m_pWidget, &CSceneNodeModelWidget::SignalAddScriptFileClicked,
          this, &CSceneNodeModel::SignalAddScriptFileRequested);

  m_pWidget->SetScriptButtonEnabled(false);
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

    m_pWidget->SetScriptButtonEnabled(true);
    if (nullptr != m_spScene)
    {
      QReadLocker locker(&m_spScene->m_rwLock);
      if (!m_spScene->m_sScript.isEmpty())
      {
        m_pWidget->SetScriptButtonEnabled(false);
      }
    }
  }

  if (nullptr != m_pWidget)
  {
    m_pWidget->SetName(m_sSceneName);
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
    if (nullptr != m_pWidget)
    {
      m_pWidget->SetName(m_sSceneName);
    }
  }
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
  return m_pWidget;
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

      m_pWidget->SetName(sSceneNameAfterChange);
      m_sSceneName = sSceneNameAfterChange;
    }
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
      m_pWidget->SetName(sNewName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SlotResourceAdded(qint32 iProjId, const QString& sName)
{
  if (ProjectId() == iProjId && nullptr != m_spScene)
  {
    QReadLocker locker(&m_spScene->m_rwLock);
    if (m_spScene->m_sScript == sName)
    {
      m_pWidget->SetScriptButtonEnabled(false);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::SlotResourceRemoved(qint32 iProjId, const QString& sName)
{
  Q_UNUSED(sName)
  if (ProjectId() == iProjId && nullptr != m_spScene)
  {
    QReadLocker locker(&m_spScene->m_rwLock);
    if (m_spScene->m_sScript.isEmpty())
    {
      m_pWidget->SetScriptButtonEnabled(true);
    }
  }
}
