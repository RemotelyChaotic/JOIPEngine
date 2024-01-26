#include "TimelineWidgetLayer.h"
#include "CommandModifyLayerProperties.h"
#include "TimelineWidget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QMouseEvent>

#include <QtWidgets/QGraphicsEffect>

class CTimeLinewidgetLayerShadow : public QGraphicsDropShadowEffect
{
public:
  CTimeLinewidgetLayerShadow() : QGraphicsDropShadowEffect() {}

  void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

protected:
  void draw(QPainter* pPainter) override
  {
    if (m_bEnabled)
    {
      QGraphicsDropShadowEffect::draw(pPainter);
    }
    else
    {
      drawSource(pPainter);
    }
  }

  bool m_bEnabled = false;
};

//----------------------------------------------------------------------------------------
//
CTimelineWidgetLayer::CTimelineWidgetLayer(const tspSequenceLayer& spLayer, CTimelineWidget* pParent,
                                           QWidget* pWidgetParent) :
  QFrame{pWidgetParent},
  m_spLayer(spLayer),
  m_pParent(pParent)
{
  setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

  QHBoxLayout* pLayout = new QHBoxLayout(this);
  pLayout->setContentsMargins({0, 0, 0, 0});
  pLayout->setSpacing(0);

  m_pHeader = new QWidget(this);
  m_pHeader->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
  QGridLayout* pBoxLayout = new QGridLayout(m_pHeader);
  pBoxLayout->setHorizontalSpacing(0);
  pBoxLayout->setVerticalSpacing(0);
  pBoxLayout->setContentsMargins({0, 0, 0, 0});

  m_pLayerTypeCombo = new QComboBox(m_pHeader);
  m_pLayerTypeCombo->addItem(sequence::c_sCategoryIdBeat, sequence::c_sCategoryIdBeat);
  m_pLayerTypeCombo->addItem(sequence::c_sCategoryIdToy, sequence::c_sCategoryIdToy);
  m_pLayerTypeCombo->addItem(sequence::c_sCategoryIdResource, sequence::c_sCategoryIdResource);
  m_pLayerTypeCombo->addItem(sequence::c_sCategoryIdText, sequence::c_sCategoryIdText);
  m_pLayerTypeCombo->addItem(sequence::c_sCategoryIdScript, sequence::c_sCategoryIdScript);
  connect(m_pLayerTypeCombo, qOverload<qint32>(&QComboBox::currentIndexChanged),
          this, &CTimelineWidgetLayer::SlotTypeChanged);
  pBoxLayout->addWidget(m_pLayerTypeCombo, 0, 0);

  m_pNameLineEdit = new QLineEdit(m_pHeader);
  connect(m_pNameLineEdit, &QLineEdit::editingFinished,
          this, &CTimelineWidgetLayer::SlotLabelChanged);
  pBoxLayout->addWidget(m_pNameLineEdit, 1, 0);

  pBoxLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Fixed, QSizePolicy::Preferred), 2, 0);

  pLayout->addWidget(m_pHeader);

  m_pTimeLineContent = new QScrollArea(this);
  pLayout->addWidget(m_pTimeLineContent);

  m_pDropShadow = new CTimeLinewidgetLayerShadow;
  m_pDropShadow->setOffset(0, 0);
  m_pDropShadow->setBlurRadius(20);
  m_pDropShadow->setColor(Qt::white);
  m_pDropShadow->SetEnabled(false);

  setGraphicsEffect(m_pDropShadow);

  SetLayer(spLayer);
}
CTimelineWidgetLayer::~CTimelineWidgetLayer() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetLayer(const tspSequenceLayer& spLayer)
{
  m_spLayer = spLayer;
  UpdateUi();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetHighlight(QColor col, QColor alternateCol)
{
  m_pDropShadow->SetEnabled(col.isValid());
  m_pDropShadow->setColor(col);
  if (alternateCol.isValid())
  {
    setStyleSheet("CTimelineWidgetLayer {background-color:" + alternateCol.name() +";}");
  }
  else
  {
    setStyleSheet(QString());
  }
  m_alternateBgCol = alternateCol;
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetUndoStack(QPointer<QUndoStack> pUndo)
{
  m_pUndoStack = pUndo;
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
tspSequenceLayer CTimelineWidgetLayer::Layer() const
{
  return m_spLayer;
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
QSize CTimelineWidgetLayer::HeaderSize() const
{
  return m_pHeader->size();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::UpdateUi()
{
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
void CTimelineWidgetLayer::mousePressEvent(QMouseEvent* pEvent)
{
  if (m_pHeader->underMouse() && nullptr != pEvent)
  {
    m_bMousePotentionallyStartedDrag = true;
    m_dragDistance = QPoint();
    m_dragOrigin = pEvent->globalPos();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::mouseMoveEvent(QMouseEvent* pEvent)
{
  if (nullptr != pEvent)
  {
    if (m_bMousePotentionallyStartedDrag)
    {
      if (pEvent->buttons() & Qt::LeftButton)
      {
        m_dragDistance = pEvent->globalPos() - m_dragOrigin;
        if (m_dragDistance.manhattanLength() >= QApplication::startDragDistance())
        {
          emit SignalUserStartedDrag();
          m_bMousePotentionallyStartedDrag = false;
          m_dragDistance = QPoint();
        }
      }
      else
      {
        m_bMousePotentionallyStartedDrag = false;
        m_dragDistance = QPoint();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::mouseReleaseEvent(QMouseEvent*)
{
  m_bMousePotentionallyStartedDrag = false;
  m_dragDistance = QPoint();

  if (underMouse())
  {
    emit SignalSelected();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::paintEvent(QPaintEvent* pEvt)
{
  QFrame::paintEvent(pEvt);
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SlotLabelChanged()
{
  if (nullptr != m_pUndoStack && nullptr != m_spLayer)
  {
    auto spLayerOld = m_spLayer->Clone();
    auto spLayerNew = m_spLayer->Clone();
    spLayerNew->m_sName = m_pNameLineEdit->text();
    m_pUndoStack->push(new CCommandModifyLayerProperties(
        m_pParent,
        spLayerOld, spLayerNew,
        m_pParent->IndexOf(this)));
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SlotTypeChanged()
{
  if (nullptr != m_pUndoStack && nullptr != m_spLayer)
  {
    auto spLayerOld = m_spLayer->Clone();
    auto spLayerNew = m_spLayer->Clone();
    spLayerNew->m_sLayerType = m_pLayerTypeCombo->currentData().toString();
    m_pUndoStack->push(new CCommandModifyLayerProperties(
        m_pParent,
        spLayerOld, spLayerNew,
        m_pParent->IndexOf(this)));
  }
}
