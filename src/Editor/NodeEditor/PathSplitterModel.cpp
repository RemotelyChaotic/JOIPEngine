#include "PathSplitterModel.h"
#include "PathSplitterModelWidget.h"
#include "SceneTranstitionData.h"

#include "QJsonArray"

namespace {
  const qint32 c_iInPorts = 1;
  const qint32 c_iOutPorts = 4;
}


CPathSplitterModel::CPathSplitterModel() :
  NodeDataModel(),
  m_spOutData(std::make_shared<CSceneTranstitionData>()),
  m_pWidget(new CPathSplitterModelWidget()),
  m_modelValidationState(NodeValidationState::Warning),
  m_modelValidationError(QString(tr("Missing or incorrect input"))),
  m_vsLabelNames(),
  m_transitonType(ESceneTransitionType::eRandom)
{
  for (qint32 i = 0; i < c_iOutPorts; i++)
  {
    m_vsLabelNames.push_back(QString());
  }

  connect(m_pWidget, &CPathSplitterModelWidget::SignalTransitionTypeChanged,
          this, &CPathSplitterModel::SlotTransitionTypeChanged);
  connect(m_pWidget, &CPathSplitterModelWidget::SignalTransitionLabelChanged,
          this, &CPathSplitterModel::SlotTransitionLabelChanged);
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
    m_pWidget->SetTransitionType(m_transitonType._to_integral());
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
        m_pWidget->SetTransitionLabel(static_cast<qint32>(i), sLabel);
        i++;
      }
    }
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
  return m_pWidget;
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
void CPathSplitterModel::SlotTransitionTypeChanged(qint32 iType)
{
  m_transitonType = ESceneTransitionType::_from_integral(iType);
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModel::SlotTransitionLabelChanged(PortIndex index, const QString& sLabelValue)
{
  if (-1 < index && m_vsLabelNames.size() > static_cast<size_t>(index))
  {
    m_vsLabelNames[static_cast<size_t>(index)] = sLabelValue;
  }
}
