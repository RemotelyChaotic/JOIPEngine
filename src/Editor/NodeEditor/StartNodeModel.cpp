#include "StartNodeModel.h"
#include "SceneTranstitionData.h"

namespace {
  const qint32 c_iInPorts = 0;
  const qint32 c_iOutPorts = 1;
}

CStartNodeModel::CStartNodeModel() :
  NodeDataModel(),
  m_spTransition(std::make_shared<CSceneTranstitionData>()),
  m_modelValidationState(NodeValidationState::Warning),
  m_modelValidationError(QString(tr("Missing output")))
{
}

//----------------------------------------------------------------------------------------
//
QString CStartNodeModel::caption() const
{ return staticCaption(); }
bool CStartNodeModel::captionVisible() const
{ return true; }
QString CStartNodeModel::name() const
{ return staticCaption(); }

//----------------------------------------------------------------------------------------
//
QJsonObject CStartNodeModel::save() const
{
  QJsonObject modelJson = NodeDataModel::save();
  return modelJson;
}

//----------------------------------------------------------------------------------------
//
void CStartNodeModel::restore(QJsonObject const& p)
{
  Q_UNUSED(p);
}

//----------------------------------------------------------------------------------------
//
unsigned int CStartNodeModel::nPorts(PortType portType) const
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
NodeDataType CStartNodeModel::dataType(PortType, PortIndex) const
{
  return CSceneTranstitionData().type();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<NodeData> CStartNodeModel::outData(PortIndex port)
{
  Q_UNUSED(port)
  return std::static_pointer_cast<NodeData>(m_spTransition);
}

//----------------------------------------------------------------------------------------
//
NodeValidationState CStartNodeModel::validationState() const
{
  return m_modelValidationState;
}

//----------------------------------------------------------------------------------------
//
QString CStartNodeModel::validationMessage() const
{
  return m_modelValidationError;
}

//----------------------------------------------------------------------------------------
//
void CStartNodeModel::outputConnectionCreated(QtNodes::Connection const& c)
{
  Q_UNUSED(c)
  m_modelValidationState = NodeValidationState::Valid;
  m_modelValidationError = QString();
}

//----------------------------------------------------------------------------------------
//
void CStartNodeModel::outputConnectionDeleted(QtNodes::Connection const& c)
{
  Q_UNUSED(c)
  m_modelValidationState = NodeValidationState::Warning;
  m_modelValidationError = QString(tr("Missing output"));
}
