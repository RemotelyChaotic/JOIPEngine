#include "PathSplitterModel.h"

#include "Application.h"
#include "PathSplitterModelWidget.h"
#include "CommandNodeEdited.h"
#include "SceneTranstitionData.h"

#include "Systems/DatabaseManager.h"

#include <QJsonArray>

namespace {
  const qint32 c_iInPorts = 1;
  const qint32 c_iOutPorts = 4;
}


CPathSplitterModel::CPathSplitterModel() :
  CEditorNodeModelBase(),
  m_spOutData(std::make_shared<CSceneTranstitionData>()),
  m_modelValidationState(NodeValidationState::Warning),
  m_modelValidationError(QString(tr("Missing or incorrect input"))),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_vsLabelNames(),
  m_transitonType(ESceneTransitionType::eRandom)
{
  for (qint32 i = 0; i < c_iOutPorts; i++)
  {
    m_vsLabelNames.push_back(QString());
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceAdded,
            this, &CPathSplitterModel::SlotResourceAdded);
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceRenamed,
            this, &CPathSplitterModel::SlotResourceRenamed);
    connect(spDbManager.get(), &CDatabaseManager::SignalResourceRemoved,
            this, &CPathSplitterModel::SlotResourceRemoved);
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::SetProjectId(qint32 iId)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spProject = spDbManager->FindProject(iId);
    OnProjectSetImpl();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CPathSplitterModel::ProjectId()
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
QString CPathSplitterModel::TransitionLabel(PortIndex port)
{
  if (-1 < port && m_vsLabelNames.size() > static_cast<size_t>(port))
  {
    return m_vsLabelNames[static_cast<size_t>(port)];
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::SetTransitionLabel(PortIndex index, const QString& sLabelValue)
{
  SlotTransitionLabelChanged(index, sLabelValue);
}

//----------------------------------------------------------------------------------------
//
QString CPathSplitterModel::caption() const
{ return staticCaption(); }
bool CPathSplitterModel::captionVisible() const
{ return true; }
QString CPathSplitterModel::name() const
{ return staticCaption(); }

//----------------------------------------------------------------------------------------
//
QJsonObject CPathSplitterModel::save() const
{
  QJsonObject modelJson = NodeDataModel::save();
  modelJson["transitonType"] = m_transitonType._to_integral();
  QJsonArray aLabels;
  for (auto& sLabel : m_vsLabelNames)
  {
    aLabels.push_back(sLabel);
  }
  modelJson["vsLabelNames"] = aLabels;
  modelJson["bCustomLayoutEnabled"] = m_bCustomLayoutEnabled;
  modelJson["sCustomLayout"] = m_sCustomLayout;
  return modelJson;
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::restore(QJsonObject const& p)
{
  QJsonValue v = p["transitonType"];
  if (!v.isUndefined())
  {
    m_transitonType = ESceneTransitionType::_from_integral(v.toInt());
  }
  QJsonValue arr = p["vsLabelNames"];
  if (!arr.isUndefined())
  {
    size_t i = 0;
    for (QJsonValue val : arr.toArray())
    {
      if (m_vsLabelNames.size() > i)
      {
        const QString sLabel = val.toString();
        m_vsLabelNames[i] = sLabel;
        i++;
      }
    }
  }
  v = p["bCustomLayoutEnabled"];
  m_bCustomLayoutEnabled = false;
  if (!v.isUndefined())
  {
    m_bCustomLayoutEnabled = v.toBool(false);
  }
  v = p["sCustomLayout"];
  m_sCustomLayout = QString();
  if (!v.isUndefined())
  {
    m_sCustomLayout = v.toString(QString());
  }
}

//----------------------------------------------------------------------------------------
//
unsigned int CPathSplitterModel::nPorts(PortType portType) const
{
  unsigned int result = 1;

  switch (portType)
  {
    case PortType::In:
      result = c_iInPorts;
      break;
    case PortType::Out:
      result = c_iOutPorts;
    default:
      break;
  }

  return result;
}

//----------------------------------------------------------------------------------------
//
NodeDataType CPathSplitterModel::dataType(PortType, PortIndex) const
{
  return CSceneTranstitionData().type();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<NodeData> CPathSplitterModel::outData(PortIndex port)
{
  Q_UNUSED(port)
  return std::static_pointer_cast<NodeData>(m_spOutData);
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::setInData(std::shared_ptr<NodeData> data, PortIndex portIndex)
{
  auto newData =
    std::dynamic_pointer_cast<CSceneTranstitionData>(data);

  if (portIndex == 0 && nullptr != newData)
  {
    m_wpInData = newData;
    m_modelValidationState = NodeValidationState::Valid;
    m_modelValidationError = QString();
  }
}

//----------------------------------------------------------------------------------------
//
QWidget* CPathSplitterModel::embeddedWidget()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
NodeValidationState CPathSplitterModel::validationState() const
{
  return m_modelValidationState;
}

//----------------------------------------------------------------------------------------
//
QString CPathSplitterModel::validationMessage() const
{
  return m_modelValidationError;
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::SlotResourceAdded(qint32 iProjId, const QString& sName)
{
  auto spDbManager = m_wpDbManager.lock();
  if (ProjectId() == iProjId && nullptr != spDbManager)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sName);
    if (nullptr != spResource)
    {
      QReadLocker locker(&spResource->m_rwLock);
      if (EResourceType::eLayout == spResource->m_type._to_integral())
      {
        SlotResourceAddedImpl(sName, spResource->m_type);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::SlotResourceRenamed(qint32 iProjId,
                                          const QString& sOldName, const QString& sName)
{
  auto spDbManager = m_wpDbManager.lock();
  if (ProjectId() == iProjId && nullptr != spDbManager)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sName);
    if (nullptr != spResource)
    {
      QReadLocker locker(&spResource->m_rwLock);
      if (EResourceType::eLayout == spResource->m_type._to_integral())
      {
        SlotResourceRenamedImpl(sOldName, sName, spResource->m_type);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::SlotResourceRemoved(qint32 iProjId, const QString& sName)
{
  auto spDbManager = m_wpDbManager.lock();
  if (ProjectId() == iProjId && nullptr != spDbManager)
  {
    // the resource is already removed, just remove from both lists, since names
    // are unique
    SlotResourceRemovedImpl(sName, EResourceType::eLayout);
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::SlotCustomTransitionChanged(bool bEnabled, const QString& sResource)
{
  QJsonObject oldState = save();
  m_bCustomLayoutEnabled = bEnabled;
  m_sCustomLayout = sResource;
  QJsonObject newState = save();

  if (nullptr != UndoStack())
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::SlotTransitionTypeChanged(qint32 iType)
{
  QJsonObject oldState = save();
  m_transitonType = ESceneTransitionType::_from_integral(iType);
  QJsonObject newState = save();

  if (nullptr != UndoStack())
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::SlotTransitionLabelChanged(PortIndex index, const QString& sLabelValue)
{
  QJsonObject oldState = save();
  if (-1 < index && m_vsLabelNames.size() > static_cast<size_t>(index))
  {
    m_vsLabelNames[static_cast<size_t>(index)] = sLabelValue;
  }
  QJsonObject newState = save();

  if (nullptr != UndoStack())
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
CPathSplitterModelWithWidget::CPathSplitterModelWithWidget() :
  CPathSplitterModel(),
  m_pWidget(new CPathSplitterModelWidget())
{
  connect(m_pWidget, &CPathSplitterModelWidget::SignalAddLayoutFileClicked,
          this, &CPathSplitterModelWithWidget::SignalAddLayoutFileRequested);
  connect(m_pWidget, &CPathSplitterModelWidget::SignalCustomTransitionChanged,
          this, &CPathSplitterModelWithWidget::SlotCustomTransitionChanged);
  connect(m_pWidget, &CPathSplitterModelWidget::SignalTransitionTypeChanged,
          this, &CPathSplitterModelWithWidget::SlotTransitionTypeChanged);
  connect(m_pWidget, &CPathSplitterModelWidget::SignalTransitionLabelChanged,
          this, &CPathSplitterModelWithWidget::SlotTransitionLabelChanged);
}
CPathSplitterModelWithWidget::~CPathSplitterModelWithWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWithWidget::restore(QJsonObject const& p)
{
  CPathSplitterModel::restore(p);

  m_pWidget->SetTransitionType(m_transitonType._to_integral());

  for (qint32 i = 0; static_cast<qint32>(m_vsLabelNames.size()) > i; ++i)
  {
    m_pWidget->SetTransitionLabel(static_cast<qint32>(i), m_vsLabelNames[i]);
  }

  m_pWidget->SetCustomLayout(m_bCustomLayoutEnabled, m_sCustomLayout);
}

//----------------------------------------------------------------------------------------
//
QWidget* CPathSplitterModelWithWidget::embeddedWidget()
{
  return m_pWidget;
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWithWidget::OnProjectSetImpl()
{
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetProject(m_spProject);
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWithWidget::OnUndoStackSet()
{
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetUndoStack(m_pUndoStack);
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWithWidget::SlotResourceAddedImpl(const QString& sName, EResourceType type)
{
  if (nullptr != m_pWidget)
  {
    if (EResourceType::eLayout == type._to_integral())
    {
      m_pWidget->OnLayoutAdded(sName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWithWidget::SlotResourceRenamedImpl(const QString& sOldName,
                                                        const QString& sName,
                                                        EResourceType type)
{
  if (nullptr != m_pWidget)
  {
    if (EResourceType::eLayout == type._to_integral())
    {
      m_pWidget->OnLayoutRenamed(sOldName, sName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWithWidget::SlotResourceRemovedImpl(const QString& sName,
                                                        EResourceType type)
{
  if (nullptr != m_pWidget)
  {
    if (EResourceType::eLayout == type._to_integral())
    {
      m_pWidget->OnLayoutRemoved(sName);
    }
  }
}
