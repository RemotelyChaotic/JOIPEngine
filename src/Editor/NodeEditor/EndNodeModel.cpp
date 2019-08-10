#include "EndNodeModel.h"
#include "SceneTranstitionData.h"

namespace {
  const qint32 c_iInPorts = 1;
  const qint32 c_iOutPorts = 0;
}


CEndNodeModel::CEndNodeModel() :
  NodeDataModel(),
  m_modelValidationState(NodeValidationState::Warning),
  m_modelValidationError(QString(tr("Missing or incorrect inputs")))
{
}

//----------------------------------------------------------------------------------------
//
QString CEndNodeModel::caption() const
{ return QStringLiteral("Exit Point"); }
bool CEndNodeModel::captionVisible() const
{ return true; }
QString CEndNodeModel::name() const
{ return tr("Exit Point"); }

//----------------------------------------------------------------------------------------
//
QJsonObject CEndNodeModel::save() const
{
  QJsonObject modelJson = NodeDataModel::save();
  return modelJson;
}

//----------------------------------------------------------------------------------------
//
void CEndNodeModel::restore(QJsonObject const& p)
{
  Q_UNUSED(p);
}

//----------------------------------------------------------------------------------------
//
unsigned int CEndNodeModel::nPorts(PortType portType) const
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
NodeDataType CEndNodeModel::dataType(PortType, PortIndex) const
{
  return CSceneTranstitionData().type();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<NodeData> CEndNodeModel::outData(PortIndex port)
{
  Q_UNUSED(port)
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CEndNodeModel::setInData(std::shared_ptr<NodeData> data, PortIndex portIndex)
{
  auto newData =
    std::dynamic_pointer_cast<CSceneTranstitionData>(data);

  if (portIndex == 0 && nullptr != newData)
  {
    m_wpTransition = newData;
    m_modelValidationState = NodeValidationState::Valid;
    m_modelValidationError = QString();
  }
}

//----------------------------------------------------------------------------------------
//
NodeValidationState CEndNodeModel::validationState() const
{
  return m_modelValidationState;
}

//----------------------------------------------------------------------------------------
//
QString CEndNodeModel::validationMessage() const
{
  return m_modelValidationError;
}
