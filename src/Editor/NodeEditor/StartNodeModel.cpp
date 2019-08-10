#include "StartNodeModel.h"
#include "SceneTranstitionData.h"

namespace {
  const qint32 c_iInPorts = 0;
  const qint32 c_iOutPorts = 1;
}

CStartNodeModel::CStartNodeModel() :
  NodeDataModel(),
  m_spTransition(std::make_shared<CSceneTranstitionData>())
{
}

//----------------------------------------------------------------------------------------
//
QString CStartNodeModel::caption() const
{ return QStringLiteral("Entry Point"); }
bool CStartNodeModel::captionVisible() const
{ return true; }
QString CStartNodeModel::name() const
{ return tr("Entry Point"); }

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
