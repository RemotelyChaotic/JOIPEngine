#include "SceneNodeModel.h"
#include "SceneNodeModelWidget.h"
#include "SceneTranstitionData.h"

namespace {
  const qint32 c_iInPorts = 1;
  const qint32 c_iOutPorts = 1;
}

CSceneNodeModel::CSceneNodeModel() :
  NodeDataModel(),
  m_spOutData(std::make_shared<CSceneTranstitionData>()),
  m_pWidget(new CSceneNodeModelWidget()),
  m_modelValidationState(NodeValidationState::Warning),
  m_modelValidationError(QString(tr("Missing or incorrect inputs")))
{
  //connect(m_pWidget, &CSceneNodeModelWidget::SignalNumberOfOutputsChanged,
  //        this, &CSceneNodeModel::SlotNumberOfOutputsChanged);
}

//----------------------------------------------------------------------------------------
//
QString CSceneNodeModel::caption() const
{
  return QString("Scene");
}

//----------------------------------------------------------------------------------------
//
QString CSceneNodeModel::name() const
{
  return  QString(tr("Scene"));
}

//----------------------------------------------------------------------------------------
//
QJsonObject CSceneNodeModel::save() const
{
  QJsonObject modelJson = NodeDataModel::save();
  return modelJson;
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModel::restore(QJsonObject const& p)
{
  Q_UNUSED(p);
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
    m_modelValidationState = NodeValidationState::Valid;
    m_modelValidationError = QString();

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
void CSceneNodeModel::SlotNumberOfOutputsChanged(qint32 iValue)
{
  Q_UNUSED(iValue);
}
