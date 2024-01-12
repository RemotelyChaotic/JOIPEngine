#include "TimelineWidgetLayer.h"

#include <QHBoxLayout>
#include <QLineEdit>

CTimelineWidgetLayer::CTimelineWidgetLayer(const tspSequenceLayer& spLayer, QWidget* pParent) :
  QWidget{pParent},
  m_spLayer(spLayer)
{
  QHBoxLayout* pLayout = new QHBoxLayout(this);
  pLayout->setContentsMargins({0, 0, 0, 0});

  QWidget* pHeader = new QWidget(this);
  QGridLayout* pBoxLayout = new QGridLayout(pHeader);
  pBoxLayout->setHorizontalSpacing(0);
  pBoxLayout->setVerticalSpacing(0);
  pBoxLayout->setContentsMargins({0, 0, 0, 0});

  m_pLayerTypeCombo = new QComboBox(pHeader);
  m_pLayerTypeCombo->addItem(sequence::c_sCategoryIdBeat, sequence::c_sCategoryIdBeat);
  m_pLayerTypeCombo->addItem(sequence::c_sCategoryIdToy, sequence::c_sCategoryIdToy);
  m_pLayerTypeCombo->addItem(sequence::c_sCategoryIdResource, sequence::c_sCategoryIdResource);
  m_pLayerTypeCombo->addItem(sequence::c_sCategoryIdText, sequence::c_sCategoryIdText);
  m_pLayerTypeCombo->addItem(sequence::c_sCategoryIdScript, sequence::c_sCategoryIdScript);
  pBoxLayout->addWidget(m_pLayerTypeCombo, 0, 0);

  m_pNameLineEdit = new QLineEdit(pHeader);
  pBoxLayout->addWidget(m_pNameLineEdit, 1, 0);

  pBoxLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Fixed, QSizePolicy::Preferred), 2, 0);

  pLayout->addWidget(pHeader);

  m_pTimeLineContent = new QScrollArea(this);
  pLayout->addWidget(m_pTimeLineContent);

  SetLayer(spLayer);
}
CTimelineWidgetLayer::~CTimelineWidgetLayer() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetLayer(const tspSequenceLayer& spLayer)
{
  m_spLayer = spLayer;
  if (nullptr != spLayer)
  {
    QSignalBlocker b1(m_pLayerTypeCombo);
    QSignalBlocker b2(m_pNameLineEdit);
    m_pLayerTypeCombo->setCurrentIndex(m_pLayerTypeCombo->findData(spLayer->m_sLayerType, Qt::UserRole));
    m_pNameLineEdit->setText(spLayer->m_sName);
  }
}

//----------------------------------------------------------------------------------------
//
QString CTimelineWidgetLayer::Name() const
{
  if (nullptr != m_spLayer)
  {
    return m_spLayer->m_sName;
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
QString CTimelineWidgetLayer::LayerType() const
{
  if (nullptr != m_spLayer)
  {
    return m_spLayer->m_sLayerType;
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::paintEvent(QPaintEvent* pEvt)
{
  QWidget::paintEvent(pEvt);
}
