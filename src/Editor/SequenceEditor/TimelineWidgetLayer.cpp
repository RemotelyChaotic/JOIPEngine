#include "TimelineWidgetLayer.h"
#include "CommandModifyLayerProperties.h"
#include "TimelineSeqeunceInstructionConfigOverlay.h"
#include "TimelineWidget.h"
#include "TimelineWidgetLayerBackground.h"

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
                                           QWidget* pWidgetParent, QPointer<CResourceTreeItemModel> pItemModel) :
  QFrame{pWidgetParent},
  m_spLayer(spLayer),
  m_pEditorModel(pItemModel),
  m_pParent(pParent),
  m_selectionColor(Qt::white)
{
  std::sort(m_spLayer->m_vspInstructions.begin(), m_spLayer->m_vspInstructions.end(),
            [](const sequence::tTimedInstruction& a, const sequence::tTimedInstruction& b){
              return a.first < b.first;
            });

  qApp->installEventFilter(this);
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

  m_pTimeLineContent = new CTimelineWidgetLayerBackground(this);
  connect(m_pTimeLineContent, &CTimelineWidgetLayerBackground::SignalOpenInsertContextMenuAt,
          this, &CTimelineWidgetLayer::SignalOpenInsertContextMenuAt);
  connect(m_pTimeLineContent, &CTimelineWidgetLayerBackground::SignalEditInstruction,
          this, &CTimelineWidgetLayer::SlotSelectedInstruction);
  pLayout->addWidget(m_pTimeLineContent);

  m_pDropShadow = new CTimeLinewidgetLayerShadow;
  m_pDropShadow->setOffset(0, 0);
  m_pDropShadow->setBlurRadius(20);
  m_pDropShadow->setColor(m_selectionColor);
  m_pDropShadow->SetEnabled(false);

  m_pConfigOverlay = new CTimelineSeqeunceInstructionConfigOverlay(this);
  m_pConfigOverlay->Hide();
  connect(m_pConfigOverlay, &CTimelineSeqeunceInstructionConfigOverlay::SignalCurrentItemChanged,
          this, &CTimelineWidgetLayer::SlotInstructionChanged);

  setGraphicsEffect(m_pDropShadow);

  SetLayer(spLayer);
}
CTimelineWidgetLayer::~CTimelineWidgetLayer() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetGridColor(const QColor& col)
{
  m_pTimeLineContent->SetGridColor(col);
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetLayer::GridColor() const
{
  return m_pTimeLineContent->GridColor();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetOutOfRangeColor(const QColor& col)
{
  m_pTimeLineContent->SetOutOfRangeColor(col);
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetLayer::OutOfRangeColor() const
{
  return m_pTimeLineContent->OutOfRangeColor();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetSelectionColor(const QColor& col)
{
  m_selectionColor = col;
  m_pDropShadow->setColor(m_selectionColor);
  m_pTimeLineContent->SetSelectionColor(col);
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetLayer::SelectionColor() const
{
  return m_selectionColor;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetTimelineBackgroundColor(const QColor& col)
{
  m_pTimeLineContent->SetTimelineBackgroundColor(col);
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetLayer::TimelineBackgroundColor() const
{
  return m_pTimeLineContent->TimelineBackgroundColor();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::AddNewElement(const QString& sId, qint64 iTimestamp)
{
  QString sDescr = QString("added instruction '%2'").arg(sId);
  auto spLayerOld = m_spLayer->Clone();
  auto spLayerNew = m_spLayer->Clone();
  if (nullptr == m_pTimeLineContent->InstructionFromTime(iTimestamp))
  {
    spLayerNew->m_vspInstructions.push_back(
        {iTimestamp, sequence::CreateInstruction(sId, QVariantList())});
    std::sort(spLayerNew->m_vspInstructions.begin(), spLayerNew->m_vspInstructions.end(),
              [](const sequence::tTimedInstruction& a, const sequence::tTimedInstruction& b){
                return a.first < b.first;
              });
    m_pUndoStack->push(new CCommandAddOrRemoveInstruction(
        m_pParent,
        spLayerOld, spLayerNew,
        m_pParent->IndexOf(this),
        sDescr));

    m_pTimeLineContent->SetSelectedInstruction(-1);
    m_pTimeLineContent->SetSelectedInstruction(iTimestamp);

    emit SignalSelected();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::ClearSelection()
{
  m_pTimeLineContent->ClearSelection();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::CloseConfigOverlay()
{
  m_pConfigOverlay->Hide();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::RemoveSelectedElement()
{
  auto spLayerOld = m_spLayer->Clone();
  auto spLayerNew = m_spLayer->Clone();
  qint64 iTime = m_pTimeLineContent->CurrentlySelectedInstructionTime();
  auto it = std::find_if(spLayerNew->m_vspInstructions.begin(), spLayerNew->m_vspInstructions.end(),
                         [&iTime](const sequence::tTimedInstruction& pair) {
                           return iTime == pair.first;
                         });
  if (-1 != iTime && spLayerNew->m_vspInstructions.end() != it)
  {
    QString sDescr = QString("removed instruction '%2'").arg(it->second->m_sInstructionType);
    spLayerNew->m_vspInstructions.erase(it);
    std::sort(spLayerNew->m_vspInstructions.begin(), spLayerNew->m_vspInstructions.end(),
              [](const sequence::tTimedInstruction& a, const sequence::tTimedInstruction& b){
                return a.first < b.first;
              });
    m_pUndoStack->push(new CCommandAddOrRemoveInstruction(
        m_pParent,
        spLayerOld, spLayerNew,
        m_pParent->IndexOf(this),
        sDescr));
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetLayer(const tspSequenceLayer& spLayer)
{
  m_spLayer = spLayer;
  m_pTimeLineContent->SetLayer(spLayer);
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
void CTimelineWidgetLayer::SetTimeGrid(qint64 iGrid)
{
  m_pTimeLineContent->SetTimeGrid(iGrid);
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
QPointer<CResourceTreeItemModel> CTimelineWidgetLayer::ResourceItemModel() const
{
  return m_pEditorModel;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::UpdateUi()
{
  if (nullptr != m_spLayer)
  {
    QSignalBlocker b1(m_pLayerTypeCombo);
    QSignalBlocker b2(m_pNameLineEdit);
    m_pLayerTypeCombo->setCurrentIndex(m_pLayerTypeCombo->findData(m_spLayer->m_sLayerType, Qt::UserRole));
    m_pNameLineEdit->setText(m_spLayer->m_sName);
    m_pTimeLineContent->repaint();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetCurrentWindow(qint64 iStartMs, qint64 iPageLengthMs)
{
  m_pTimeLineContent->SetCurrentWindow(iStartMs, iPageLengthMs);
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SetTimeMaximum(qint64 iTimeMs)
{
  m_pTimeLineContent->SetTimeMaximum(iTimeMs);
}

//----------------------------------------------------------------------------------------
//
bool CTimelineWidgetLayer::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (auto pWidget = qobject_cast<QWidget*>(pObj);
      nullptr != pWidget && nullptr != pEvt && pEvt->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* pMouseEvt = static_cast<QMouseEvent*>(pEvt);
    if (Qt::MouseButton::LeftButton == pMouseEvt->button() ||
        Qt::MouseButton::RightButton == pMouseEvt->button())
    {
      if (!m_pConfigOverlay->isAncestorOf(pWidget) && m_pConfigOverlay->IsForcedOpen())
      {
        m_pConfigOverlay->Hide();
        return false;
      }
    }
  }
  return false;
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
void CTimelineWidgetLayer::resizeEvent(QResizeEvent*)
{
  if (m_pConfigOverlay->isVisible())
  {
    m_pConfigOverlay->Resize();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SlotInstructionChanged()
{
  auto spLayerOld = m_spLayer->Clone();
  auto spLayerNew = m_spLayer->Clone();
  qint64 iTime = m_pTimeLineContent->CurrentlySelectedInstructionTime();
  auto it = std::find_if(spLayerNew->m_vspInstructions.begin(), spLayerNew->m_vspInstructions.end(),
                         [&iTime](const sequence::tTimedInstruction& pair) {
                           return iTime == pair.first;
                         });
  if (-1 != iTime && spLayerNew->m_vspInstructions.end() != it)
  {
    QString sDescr = QString("changed instruction '%2'").arg(it->second->m_sInstructionType);
    *it->second = *(m_pConfigOverlay->CurrentInstructionParameters());
    m_pUndoStack->push(new CCommandChangeInstructionParameters(
        m_pParent,
        spLayerOld, spLayerNew,
        m_pParent->IndexOf(this),
        iTime,
        sDescr));
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SlotLabelChanged()
{
  if (nullptr != m_pUndoStack && nullptr != m_spLayer)
  {
    QString sDescr = QString("%1 -> %2").arg(m_spLayer->m_sName);
    auto spLayerOld = m_spLayer->Clone();
    auto spLayerNew = m_spLayer->Clone();
    spLayerNew->m_sName = m_pNameLineEdit->text();
    sDescr = sDescr.arg(spLayerNew->m_sName);
    m_pUndoStack->push(new CCommandModifyLayerProperties(
        m_pParent,
        spLayerOld, spLayerNew,
        m_pParent->IndexOf(this),
        sDescr));
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SlotSelectedInstruction(qint64 iInstr)
{
  auto spInstr = m_pTimeLineContent->InstructionFromTime(iInstr);
  if (nullptr != spInstr)
  {
    m_pConfigOverlay->Show(iInstr, spInstr);
    emit SignalSelected();
  }
  else
  {
    m_pConfigOverlay->Hide();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayer::SlotTypeChanged()
{
  if (nullptr != m_pUndoStack && nullptr != m_spLayer)
  {
    QString sDescr = QString("%1 -> %2").arg(m_spLayer->m_sLayerType);
    auto spLayerOld = m_spLayer->Clone();
    auto spLayerNew = m_spLayer->Clone();
    spLayerNew->m_sLayerType = m_pLayerTypeCombo->currentData().toString();
    sDescr = sDescr.arg(spLayerNew->m_sLayerType);
    m_pUndoStack->push(new CCommandModifyLayerProperties(
        m_pParent,
        spLayerOld, spLayerNew,
        m_pParent->IndexOf(this),
        sDescr));
  }
}
