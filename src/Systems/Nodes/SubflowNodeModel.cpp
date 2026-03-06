#include "SubflowNodeModel.h"
#include "Application.h"
#include "SceneTranstitionData.h"
#include "SubflowNodeModelWidget.h"

#include "Systems/DatabaseManager.h"
#include "Systems/Database/Project.h"

namespace {
  const qint32 c_iInPorts = 1;
  const qint32 c_iOutPorts = 1;
}

CSubflowNodeModel::CSubflowNodeModel() :
    CNodeModelBase(),
    m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
    m_spOutData(std::make_shared<CSceneTranstitionData>()),
    m_spProject(nullptr),
    m_bOutConnected(false),
    m_modelValidationState(NodeValidationState::Warning),
    m_modelValidationError(QString(tr("Missing or incorrect inputs or output")))
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceAdded,
            this, &CSubflowNodeModel::SlotResourceAdded);
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceRenamed,
            this, &CSubflowNodeModel::SlotResourceRenamed);
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceRemoved,
            this, &CSubflowNodeModel::SlotResourceRemoved);
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::SetProjectId(qint32 iId)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spProject = spDbManager->FindProject(iId);
    ProjectSetImpl();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CSubflowNodeModel::ProjectId()
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
void CSubflowNodeModel::SetNodeName(const QString& sName)
{
  SlotNameChanged(sName);
}

//----------------------------------------------------------------------------------------
//
QString CSubflowNodeModel::caption() const
{
  return staticCaption();
}

//----------------------------------------------------------------------------------------
//
QString CSubflowNodeModel::name() const
{
  return staticCaption();
}

//----------------------------------------------------------------------------------------
//
QJsonObject CSubflowNodeModel::save() const
{
  QJsonObject modelJson = NodeDataModel::save();
  modelJson["sName"] = m_sNodeName;
  modelJson["sFlow"] = m_sFlow;
  return modelJson;
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::restore(QJsonObject const& p)
{
  QJsonValue v = p["sName"];
  if (!v.isUndefined())
  {
    m_sNodeName = v.toString();
  }
  v = p["sFlow"];
  if (!v.isUndefined())
  {
    m_sFlow = v.toString();
  }
}

//----------------------------------------------------------------------------------------
//
unsigned int CSubflowNodeModel::nPorts(PortType portType) const
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
NodeDataType CSubflowNodeModel::dataType(PortType portType, PortIndex portIndex) const
{
  Q_UNUSED(portType)
  Q_UNUSED(portIndex)
  return CSceneTranstitionData().type();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<NodeData> CSubflowNodeModel::outData(PortIndex port)
{
  Q_UNUSED(port)
  return std::static_pointer_cast<NodeData>(m_spOutData);
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::setInData(std::shared_ptr<NodeData> data, PortIndex portIndex)
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
QWidget* CSubflowNodeModel::embeddedWidget()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
NodeValidationState CSubflowNodeModel::validationState() const
{
  return m_modelValidationState;
}

//----------------------------------------------------------------------------------------
//
QString CSubflowNodeModel::validationMessage() const
{
  return m_modelValidationError;
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::outputConnectionCreated(QtNodes::Connection const& c)
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
void CSubflowNodeModel::outputConnectionDeleted(QtNodes::Connection const& c)
{
  Q_UNUSED(c)
  m_bOutConnected = false;
  m_modelValidationState = NodeValidationState::Warning;
  m_modelValidationError = QString(tr("Missing or incorrect inputs or output"));
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::SlotFlowChanged(const QString& sName)
{
  bool bChanged = m_sFlow != sName;
  if (bChanged)
  {
    SlotFlowChangedImpl(sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::SlotNameChanged(const QString& sName)
{
  bool bChanged = m_sNodeName != sName;
  if (bChanged)
  {
    SlotNameChangedImpl(sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::SlotResourceAdded(qint32 iProjId, const QString& sName)
{
  auto spDbManager = m_wpDbManager.lock();
  if (ProjectId() == iProjId && nullptr != spDbManager)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sName);
    if (nullptr != spResource)
    {
      QReadLocker locker(&spResource->m_rwLock);
      if (EResourceType::eFlow == spResource->m_type._to_integral())
      {
        SlotResourceAddedImpl(sName, spResource->m_type);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::SlotResourceRenamed(qint32 iProjId,
                                            const QString& sOldName, const QString& sName)
{
  auto spDbManager = m_wpDbManager.lock();
  if (ProjectId() == iProjId && nullptr != spDbManager)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sName);
    if (nullptr != spResource)
    {
      QReadLocker locker(&spResource->m_rwLock);
      if (EResourceType::eFlow == spResource->m_type._to_integral())
      {
        SlotResourceRenamedImpl(sOldName, sName, spResource->m_type);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::SlotResourceRemoved(qint32 iProjId, const QString& sName)
{
  auto spDbManager = m_wpDbManager.lock();
  if (ProjectId() == iProjId && nullptr != spDbManager)
  {
    // the resource is already removed, just remove from both lists, since names
    // are unique
    SlotResourceRemovedImpl(sName, EResourceType::eFlow);
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::SlotFlowChangedImpl(const QString& sName)
{
  m_sFlow = sName;
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModel::SlotNameChangedImpl(const QString& sName)
{
  m_sNodeName = sName;
}

//----------------------------------------------------------------------------------------
//
CSubflowNodeModelWithWidget::CSubflowNodeModelWithWidget() :
  CSubflowNodeModel(),
  m_pWidget(new CSubflowNodeModelWidget())
{
  connect(m_pWidget, &CSubflowNodeModelWidget::SignalAddNodeFileClicked,
          this, &CSubflowNodeModelWithWidget::SignalAddNodeFileRequested);
  connect(m_pWidget, &CSubflowNodeModelWidget::SignalFlowChanged,
          this, &CSubflowNodeModelWithWidget::SlotFlowChanged);
  connect(m_pWidget, &CSubflowNodeModelWidget::SignalNameChanged,
          this, &CSubflowNodeModelWithWidget::SlotNameChanged);
}
CSubflowNodeModelWithWidget::~CSubflowNodeModelWithWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWithWidget::SetProjectId(qint32 iId)
{
  CSubflowNodeModel::SetProjectId(iId);
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetNodeName(m_sNodeName);
    m_pWidget->SetFlow(m_sFlow);
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWithWidget::restore(QJsonObject const& p)
{
  CSubflowNodeModel::restore(p);

  if (nullptr != m_pWidget)
  {
    m_pWidget->SetNodeName(m_sNodeName);
    m_pWidget->SetFlow(m_sFlow);
  }
}

//----------------------------------------------------------------------------------------
//
QWidget* CSubflowNodeModelWithWidget::embeddedWidget()
{
  return m_pWidget;
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWithWidget::ProjectSetImpl()
{
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetProject(m_spProject);
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWithWidget::SlotFlowChangedImpl(const QString& sName)
{
  CSubflowNodeModel::SlotFlowChangedImpl(sName);

  if (nullptr != m_pWidget)
  {
    m_pWidget->SetFlow(sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWithWidget::SlotNameChangedImpl(const QString& sName)
{
  CSubflowNodeModel::SlotNameChangedImpl(sName);

  if (nullptr != m_pWidget)
  {
    m_pWidget->SetNodeName(sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWithWidget::SlotResourceAddedImpl(const QString& sName, EResourceType type)
{
  if (nullptr != m_pWidget)
  {
    if (EResourceType::eFlow == type._to_integral())
    {
      m_pWidget->OnFlowAdded(sName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWithWidget::SlotResourceRenamedImpl(const QString& sOldName,
                                                          const QString& sName,
                                                          EResourceType type)
{
  if (nullptr != m_pWidget)
  {
    if (EResourceType::eFlow == type._to_integral())
    {
      m_pWidget->OnFlowRenamed(sOldName, sName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWithWidget::SlotResourceRemovedImpl(const QString& sName,
                                                          EResourceType type)
{
  if (nullptr != m_pWidget)
  {
    if (EResourceType::eFlow == type._to_integral())
    {
      m_pWidget->OnFlowRemoved(sName);
    }
  }
}
