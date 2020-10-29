#include "PathMergerModel.h"
#include "SceneTranstitionData.h"

namespace {
  const qint32 c_iInPorts = 4;
  const qint32 c_iOutPorts = 1;
}


CPathMergerModel::CPathMergerModel() :
  NodeDataModel(),
  m_spOutData(std::make_shared<CSceneTranstitionData>()),
  m_modelValidationState(NodeValidationState::Warning),
  m_modelValidationError(QString(tr("Missing or incorrect input")))
{
}

//----------------------------------------------------------------------------------------
//
QString CPathMergerModel::caption() const
{ return staticCaption(); }
bool CPathMergerModel::captionVisible() const
{ return true; }
QString CPathMergerModel::name() const
{ return staticCaption(); }

//----------------------------------------------------------------------------------------
//
QJsonObject CPathMergerModel::save() const
{
  QJsonObject modelJson = NodeDataModel::save();
  return modelJson;
}

//----------------------------------------------------------------------------------------
//
void CPathMergerModel::restore(QJsonObject const& p)
{
  Q_UNUSED(p);
}

//----------------------------------------------------------------------------------------
//
unsigned int CPathMergerModel::nPorts(PortType portType) const
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
NodeDataType CPathMergerModel::dataType(PortType, PortIndex) const
{
  return CSceneTranstitionData().type();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<NodeData> CPathMergerModel::outData(PortIndex port)
{
  Q_UNUSED(port)
  return std::static_pointer_cast<NodeData>(m_spOutData);
}

//----------------------------------------------------------------------------------------
//
void CPathMergerModel::setInData(std::shared_ptr<NodeData> data, PortIndex portIndex)
{
  auto newData =
    std::dynamic_pointer_cast<CSceneTranstitionData>(data);

  if (nullptr != newData)
  {
    switch(portIndex)
    {
      case 0:
        m_wpInData1 = newData;
        break;
      case 1:
        m_wpInData2 = newData;
        break;
      case 2:
        m_wpInData3 = newData;
        break;
      case 3:
        m_wpInData4 = newData;
        break;
    }

    if (!m_wpInData1.expired() || !m_wpInData2.expired() ||
        !m_wpInData3.expired() || !m_wpInData4.expired())
    {
      m_modelValidationState = NodeValidationState::Valid;
      m_modelValidationError = QString();
    }
  }
}

//----------------------------------------------------------------------------------------
//
NodeValidationState CPathMergerModel::validationState() const
{
  return m_modelValidationState;
}

//----------------------------------------------------------------------------------------
//
QString CPathMergerModel::validationMessage() const
{
  return m_modelValidationError;
}

