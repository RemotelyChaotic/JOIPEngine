#include "TimelineWidget.h"
#include "CommandModifyLayers.h"
#include "TimelineWidgetBackground.h"
#include "TimelineWidgetControls.h"
#include "TimelineWidgetLayer.h"
#include "TimelineWidgetOverlay.h"
#include "ui_TimelineWidget.h"

#include "Editor/SequenceEditor/SequenceEmentList.h"

#include "Widgets/SearchWidget.h"

#include <QDebug>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidgetAction>

namespace
{
  const char c_sDefaultName[] = "New Layer";
}

//----------------------------------------------------------------------------------------
//
class QTimelineWidgetLayerMineData : public QMimeData
{
  Q_OBJECT
public:
  QTimelineWidgetLayerMineData() : QMimeData() {}
  ~QTimelineWidgetLayerMineData() override {}

  void SetLayer(tspSequenceLayer spLayer) { m_spLayer = spLayer; }
  tspSequenceLayer Layer() const { return m_spLayer; }

  void SetIndex(qint32 iIndex) { m_iIndex = iIndex; }
  qint32 Index() const { return m_iIndex; }

private:
  tspSequenceLayer m_spLayer;
  qint32           m_iIndex;
};

//----------------------------------------------------------------------------------------
//
CTimelineWidget::CTimelineWidget(QWidget* pParent) :
  QScrollArea(pParent),
  m_spUi(std::make_unique<Ui::CTimelineWidget>()),
  m_pOverlay(new CTimelineWidgetOverlay(this)),
  m_pControls(new CTimelineWidgetControls(this)),
  m_selectionColor(Qt::white)
{
  m_spUi->setupUi(this);
  m_pOverlay->move(0, 0);
  m_pOverlay->resize(size());

  m_pControls->move(0, 0);
  m_pControls->resize(width(), m_pControls->minimumSizeHint().height());
  m_pControls->raise();
  connect(m_pControls, &CTimelineWidgetControls::SignalGridChanged,
          this, &CTimelineWidget::SlotTimeGridChanged);
  connect(m_pControls, &CTimelineWidgetControls::SignalZoomChanged,
          this, &CTimelineWidget::SlotZoomChanged);

  m_pCustomScrollbar = new QScrollBar(Qt::Orientation::Horizontal, this);
  m_pCustomScrollbar->resize(width(), horizontalScrollBar()->height());
  m_pCustomScrollbar->move(0, height() - m_pCustomScrollbar->height());

  setAcceptDrops(true);
  setMouseTracking(true);
  viewport()->setMouseTracking(true);

  // we will use the scrollbar for our own purposes, so disconnect it for now
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  horizontalScrollBar()->disconnect();
  connect(m_pCustomScrollbar, &QScrollBar::valueChanged,
          this, &CTimelineWidget::SlotScrollbarValueChanged);

  SlotZoomChanged(m_pControls->Zoom());

  setViewportMargins(0, m_pControls->height(), 0, m_pCustomScrollbar->height());

  connect(this, &CTimelineWidget::SignalSelectionColorChanged,
          this, &CTimelineWidget::SlotSelectionColorChanged);
}

CTimelineWidget::~CTimelineWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::setWidget(QWidget* pWidget)
{
  QScrollArea::setWidget(pWidget);
  pWidget->setAutoFillBackground(false);
  if (auto pBg = dynamic_cast<CTimelineWidgetBackground*>(pWidget))
  {
    pBg->SetTimelineWidget(this);
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetAlternateBackgroundColor(const QColor& col)
{
  m_alternateBgColor = col;
  emit SignalAlternateBackgroundColorChanged();
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidget::AlternateBackgroundColor() const
{
  return m_alternateBgColor;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetDropIndicationColor(const QColor& col)
{
  m_pOverlay->SetDropIndicationColor(col);
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidget::DropIndicationColor() const
{
  return m_pOverlay->DropIndicationColor();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetGridColor(const QColor& col)
{
  m_pControls->SetGridColor(col);
  ForAllLayers([&col](CTimelineWidgetLayer* pLayer, qint32){
    pLayer->SetGridColor(col);
  });
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidget::GridColor() const
{
  return m_pControls->GridColor();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetOutOfRangeColor(const QColor& col)
{
  m_pControls->SetOutOfRangeColor(col);
  ForAllLayers([&col](CTimelineWidgetLayer* pLayer, qint32){
    pLayer->SetOutOfRangeColor(col);
  });
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidget::OutOfRangeColor() const
{
  return m_pControls->OutOfRangeColor();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetSelectionColor(const QColor& col)
{
  m_selectionColor = col;
  emit SignalSelectionColorChanged();
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidget::SelectionColor() const
{
  return m_selectionColor;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::AddNewLayer()
{
  if (nullptr != m_pUndoStack && nullptr != m_spCurrentSequence)
  {
    qint32 iSelectedIndex =
        0 > m_iSelectedIndex ?
            static_cast<qint32>(m_spCurrentSequence->m_vspLayers.size()) : m_iSelectedIndex + 1;
    std::shared_ptr<SSequenceLayer> spLayer = std::make_shared<SSequenceLayer>();
    spLayer->m_sLayerType = sequence::c_sCategoryIdBeat;
    spLayer->m_sName = c_sDefaultName;
    m_pUndoStack->push(
        new CCommandAddOrRemoveSequenceLayer(
            this, iSelectedIndex, spLayer,
            [this](qint32 iIdx, const tspSequenceLayer& spLayer, tspSequence& spSequence){
              AddLayerTo(iIdx, spLayer, spSequence);
            },
            [this](qint32 iIdx, const tspSequenceLayer&, tspSequence& spSequence) {
              RemoveLayerFrom(iIdx, spSequence);
            }, "Add new"));
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::AddNewElement(const QString& sId, qint32 iLayer, qint64 iTimestamp)
{
  auto pLayer = Layer(iLayer);
  if (nullptr != pLayer)
  {
    pLayer->AddNewElement(sId, iTimestamp);
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::Clear()
{
  QVBoxLayout* pLayout = dynamic_cast<QVBoxLayout*>(widget()->layout());
  if (nullptr != pLayout)
  {
    while (pLayout->count() > 0)
    {
      auto pItem = pLayout->takeAt(0);
      if (nullptr != pItem->widget()) { delete pItem->widget(); }
      if (nullptr != pItem) { delete pItem; }
    }
  }
  pLayout->insertItem(0, new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}

//----------------------------------------------------------------------------------------
//
qint32 CTimelineWidget::IndexOf(CTimelineWidgetLayer* pLayer) const
{
  QLayout* pLayout = widget()->layout();
  if (nullptr != pLayout)
  {
    for (qint32 i = 0; pLayout->count() > i; ++i)
    {
      if (nullptr != pLayout->itemAt(i) && nullptr != pLayout->itemAt(i)->widget())
      {
        auto pWidget = dynamic_cast<CTimelineWidgetLayer*>(pLayout->itemAt(i)->widget());
        if (nullptr != pWidget)
        {
          if (pLayer == pWidget)
          {
            return i;
          }
        }
      }
    }
  }
  return -1;
}

//----------------------------------------------------------------------------------------
//
CTimelineWidgetLayer* CTimelineWidget::Layer(qint32 iIndex) const
{
  QLayout* pLayout = widget()->layout();
  if (nullptr != pLayout)
  {
    if (pLayout->count() > iIndex && -1 < iIndex)
    {
      if (nullptr != pLayout->itemAt(iIndex) && nullptr != pLayout->itemAt(iIndex)->widget())
      {
        return dynamic_cast<CTimelineWidgetLayer*>(pLayout->itemAt(iIndex)->widget());
      }
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
qint32 CTimelineWidget::LayerCount() const
{
  return nullptr != m_spCurrentSequence ? static_cast<qint32>(m_spCurrentSequence->m_vspLayers.size()) : 0;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::RemoveSelectedLayer()
{
  if (nullptr != m_pUndoStack && nullptr != m_spCurrentSequence)
  {
    if (0 <= m_iSelectedIndex &&
        m_spCurrentSequence->m_vspLayers.size() > static_cast<size_t>(m_iSelectedIndex))
    {
      auto spLayer = m_spCurrentSequence->m_vspLayers[static_cast<size_t>(m_iSelectedIndex)];
      m_pUndoStack->push(
          new CCommandAddOrRemoveSequenceLayer(
              this, m_iSelectedIndex, spLayer,
              [this](qint32 iIdx, const tspSequenceLayer&, tspSequence& spSequence){
                RemoveLayerFrom(iIdx, spSequence);
              },
              [this](qint32 iIdx, const tspSequenceLayer& spLayer, tspSequence& spSequence) {
                AddLayerTo(iIdx, spLayer, spSequence);
              }, "Remove"));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::RemoveSelectedElement()
{
  if (nullptr != m_spCurrentSequence)
  {
    auto pLayer = Layer(m_iSelectedIndex);
    if (nullptr != pLayer)
    {
      pLayer->RemoveSelectedElement();
    }
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CTimelineWidget::SelectedIndex() const
{
  return m_iSelectedIndex;
}

//----------------------------------------------------------------------------------------
//
qint64 CTimelineWidget::SelectedTimeStamp() const
{
  return m_pControls->CurrentTimeStamp();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetResourceModel(QPointer<CResourceTreeItemModel> pEditorModel)
{
  m_pEditorModel = pEditorModel;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetSequence(const tspSequence& spSeq)
{
  Clear();
  m_spCurrentSequence = spSeq;
  Update();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetUndoStack(QPointer<QUndoStack> pUndo)
{
  m_pUndoStack = pUndo;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::Update()
{
  if (nullptr != m_spCurrentSequence)
  {
    QVBoxLayout* pLayout = dynamic_cast<QVBoxLayout*>(widget()->layout());
    if (nullptr != pLayout)
    {
      qint32 iMax = std::max(pLayout->count()-1,
                             static_cast<qint32>(m_spCurrentSequence->m_vspLayers.size()));
      for (qint32 i = 0; iMax > i; ++i)
      {
        qint32 iCount = pLayout->count()-1;
        if (m_spCurrentSequence->m_vspLayers.size() > static_cast<size_t>(i))
        {
          auto spLayer = m_spCurrentSequence->m_vspLayers[static_cast<size_t>(i)];
          if (iCount > i)
          {
            auto pItem = pLayout->itemAt(i);
            if (nullptr != pItem)
            {
              if (auto pWidget = dynamic_cast<CTimelineWidgetLayer*>(pItem->widget()))
              {
                pWidget->SetLayer(spLayer);
              }
            }
          }
          else
          {
            pLayout->insertWidget(i, CreateLayerWidget(spLayer));
          }
        }
        else if (iCount > i)
        {
          auto pItem = pLayout->takeAt(i);
          if (nullptr != pItem->widget()) { delete pItem->widget(); }
          if (nullptr != pItem) { delete pItem; }
        }
      }
    }
  }

  // qt only recalculates layouts in the event loop and we need the layout from here on,
  // since there's no way to force it here's this badness
  QPointer<CTimelineWidget> pThis(this);
  qApp->processEvents();
  if (nullptr == pThis) { return; }

  updateGeometry();
  SlotUpdateSequenceProperties();
  SlotSelectionColorChanged();
  QMetaObject::invokeMethod(this, "SlotLayersInserted", Qt::QueuedConnection);
}

//----------------------------------------------------------------------------------------
//
tspSequence CTimelineWidget::Sequence() const
{
  return m_spCurrentSequence;
}

//----------------------------------------------------------------------------------------
//
QSize CTimelineWidget::minimumSizeHint() const
{
  return QSize(1,1);
}

//----------------------------------------------------------------------------------------
//
QSize CTimelineWidget::sizeHint() const
{
  return QScrollArea::sizeHint();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SlotOpenInsertContextMenuRequested(
    qint32 iLayerIdx, qint64 iTimestamp, QPoint globalPos)
{
  if (nullptr == m_spCurrentSequence) { return; }
  if (m_spCurrentSequence->m_vspLayers.size() <= static_cast<size_t>(iLayerIdx) || 0 > iLayerIdx) { return; }
  if (m_spCurrentSequence->m_iLengthMili <= iTimestamp || 0 > iTimestamp) { return; }

  QString sLayerType =
      m_spCurrentSequence->m_vspLayers[static_cast<size_t>(iLayerIdx)]->m_sLayerType;

  QString sId = OpenInsertContextMenuAt(QPoint(0, 0), globalPos, sLayerType);
  if (!sId.isEmpty())
  {
    AddNewElement(sId, iLayerIdx, iTimestamp);
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SlotUpdateSequenceProperties()
{
  qint64 iLength = timeline::c_iDefaultLength;
  if (nullptr != m_spCurrentSequence)
  {
    iLength = m_spCurrentSequence->m_iLengthMili;
  }

  m_pControls->SetTimeMaximum(iLength);
  ForAllLayers([&iLength](CTimelineWidgetLayer* pLayer, qint32) {
    pLayer->SetTimeMaximum(iLength);
  });

  SlotZoomChanged(m_pControls->Zoom());
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::AddLayerTo(qint32 index, const tspSequenceLayer& spLayer,
                                 tspSequence& spCurrentSequence)
{
  if (nullptr == spCurrentSequence || nullptr == spLayer) { return; }

  if (0 > index || spCurrentSequence->m_vspLayers.size() < static_cast<size_t>(index)) { return; }

  if (spCurrentSequence->m_vspLayers.size() == index)
  {
    spCurrentSequence->m_vspLayers.push_back(spLayer);
  }
  else
  {
    spCurrentSequence->m_vspLayers.insert(spCurrentSequence->m_vspLayers.begin() + index, spLayer);
  }

  QVBoxLayout* pLayout = dynamic_cast<QVBoxLayout*>(widget()->layout());
  if (nullptr != pLayout)
  {
    pLayout->insertWidget(index, CreateLayerWidget(spLayer));
    SlotSelectionColorChanged();
    QMetaObject::invokeMethod(this, "SlotLayersInserted", Qt::QueuedConnection);
  }

  emit SignalContentsChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::RemoveLayerFrom(qint32 index, tspSequence& spCurrentSequence)
{
  if (nullptr == spCurrentSequence) { return; }

  if (0 > index || spCurrentSequence->m_vspLayers.size() <= static_cast<size_t>(index)) { return; }

  spCurrentSequence->m_vspLayers.erase(spCurrentSequence->m_vspLayers.begin() + index);

  QVBoxLayout* pLayout = dynamic_cast<QVBoxLayout*>(widget()->layout());
  if (nullptr != pLayout)
  {
    auto pItem = pLayout->takeAt(index);
    if (nullptr != pItem->widget()) { delete pItem->widget(); }
    if (nullptr != pItem) { delete pItem; }
  }

  if (m_iSelectedIndex == index)
  {
    m_iSelectedIndex--;
  }

  emit SignalContentsChanged();
}

//----------------------------------------------------------------------------------------
//
bool CTimelineWidget::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pEvt && IsChildOfLayer(dynamic_cast<QWidget*>(pObj)))
  {
    switch (pEvt->type())
    {
      case QEvent::MouseMove:
      {
        QMouseEvent* pEvtM = static_cast<QMouseEvent*>(pEvt);
        m_pControls->SetCurrentCursorPos(mapFromGlobal(pEvtM->globalPos()).x());
      } break;
      case QEvent::Wheel:
      {
        QWheelEvent* pEvtM = static_cast<QWheelEvent*>(pEvt);
        wheelEvent(pEvtM);
      } break;
      case QEvent::MouseButtonRelease:
      {
        QMouseEvent* pEvtM = static_cast<QMouseEvent*>(pEvt);
        mouseReleaseEvent(pEvtM);
      } break;
      default: break;
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::dragEnterEvent(QDragEnterEvent* pEvent)
{
  pEvent->acceptProposedAction();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::dragMoveEvent(QDragMoveEvent* pEvent)
{
  pEvent->acceptProposedAction();
  m_pOverlay->UpdateDropLine();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::dropEvent(QDropEvent* pEvent)
{
  if (pEvent->source() == this && pEvent->possibleActions() & Qt::MoveAction)
  {
    if (pEvent->proposedAction() == Qt::MoveAction)
    {
      // Process the data from the event.
      const QTimelineWidgetLayerMineData *myData =
          qobject_cast<const QTimelineWidgetLayerMineData*>(pEvent->mimeData());
      if (nullptr != myData)
      {
        pEvent->acceptProposedAction();

        tspSequenceLayer spLayer = myData->Layer();
        qint32 iSourceIndex = myData->Index();
        Q_UNUSED(spLayer)
        Q_UNUSED(iSourceIndex)
      }
    }
    else
    {
      return;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::mousePressEvent(QMouseEvent* pEvent)
{
  if (nullptr != pEvent)
  {

  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::mouseMoveEvent(QMouseEvent* pEvent)
{
  if (nullptr != pEvent)
  {
    m_pControls->SetCurrentCursorPos(pEvent->pos().x());
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::mouseReleaseEvent(QMouseEvent* pEvent)
{
  if (nullptr != pEvent)
  {
    if (pEvent->button() == Qt::LeftButton)
    {
      SetSelectedTime();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::resizeEvent(QResizeEvent* pEvt)
{
  QSize s = size();
  if (verticalScrollBar()->isVisible())
  {
    s.setWidth(s.width() - verticalScrollBar()->width());
  }

  Resize(s);
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::showEvent(QShowEvent*)
{
  updateGeometry();
  SlotUpdateSequenceProperties();
  SlotSelectionColorChanged();
  SlotLayersInserted();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::wheelEvent(QWheelEvent* pEvt)
{
  if (nullptr != pEvt)
  {
    if (QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
    {
      if (pEvt->angleDelta().y() > 0)
      {
        m_pControls->ZoomIn();
      }
      else
      {
        m_pControls->ZoomOut();
      }
    }
    else
    {
      m_pCustomScrollbar->setValue(pEvt->angleDelta().y() > 0 ?
                                       m_pCustomScrollbar->value() + m_pCustomScrollbar->singleStep() :
                                       m_pCustomScrollbar->value() - m_pCustomScrollbar->singleStep());
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SlotUserStartedDrag()
{
  if (auto pWidget = qobject_cast<CTimelineWidgetLayer*>(sender()))
  {
    auto spLayer = pWidget->Layer();

    if (nullptr != m_pUndoStack && nullptr != spLayer)
    {
      QDrag* pDrag = new QDrag(this);
      QTimelineWidgetLayerMineData* pMimeData = new QTimelineWidgetLayerMineData();

      QPixmap widgetPixmap = pWidget->grab();

      qint32 iIndex = IndexOf(pWidget);
      pMimeData->SetLayer(pWidget->Layer());
      pMimeData->SetIndex(iIndex);

      pDrag->setMimeData(pMimeData);
      pDrag->setPixmap(widgetPixmap);

      m_pUndoStack->beginMacro("Moved layer " + spLayer->m_sName);
      m_pUndoStack->push(
          new CCommandAddOrRemoveSequenceLayer(
              this, iIndex, spLayer,
              [this](qint32 iIdx, const tspSequenceLayer&, tspSequence& spSequence){
                RemoveLayerFrom(iIdx, spSequence);
              },
              [this](qint32 iIdx, const tspSequenceLayer& spLayer, tspSequence& spSequence) {
                AddLayerTo(iIdx, spLayer, spSequence);
              }, "Remove"));

      m_pOverlay->SetShowDropIndicator(true);

      QPointer<CTimelineWidget> pthis(this);
      Qt::DropAction dropAction = pDrag->exec(Qt::MoveAction);
      if (nullptr == pthis || (0 == (dropAction & Qt::MoveAction))) { return; }

      qint32 iDropIndex = m_pOverlay->CurrentDropIndex();
      m_pOverlay->SetShowDropIndicator(false);
      m_pUndoStack->push(
          new CCommandAddOrRemoveSequenceLayer(
              this, iDropIndex, spLayer,
              [this](qint32 iIdx, const tspSequenceLayer& spLayer, tspSequence& spSequence){
                AddLayerTo(iIdx, spLayer, spSequence);
              },
              [this](qint32 iIdx, const tspSequenceLayer&, tspSequence& spSequence) {
                RemoveLayerFrom(iIdx, spSequence);
              }, "Add new"));
      m_pUndoStack->endMacro();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SlotLayerSelected()
{
  auto pObj = sender();
  ForAllLayers([this, &pObj](CTimelineWidgetLayer* pLayer, qint32 i) {
    auto pBg = dynamic_cast<CTimelineWidgetBackground*>(widget());
    if (nullptr != pLayer && nullptr != pBg)
    {
      if (nullptr != pObj && pObj == pLayer)
      {
        m_iSelectedIndex = i;
        pLayer->SetHighlight(SelectionColor(), AlternateBackgroundColor());
      }
      else
      {
        pLayer->ClearSelection();
        pLayer->CloseConfigOverlay();
        pLayer->SetHighlight(QColor(), QColor());
      }
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SlotLayersInserted()
{
  if (nullptr != m_spCurrentSequence && m_spCurrentSequence->m_vspLayers.size() > 0)
  {
    m_pControls->SetHeaderSize(HeadersSize());
  }
  SlotUpdateSequenceProperties();
  UpdateTimeSelectionCursor();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SlotOpenInsertContextMenuAt(QPoint p, qint64 iCursorTime)
{
  qint32 iIndex = IndexOf(qobject_cast<CTimelineWidgetLayer*>(sender()));
  SlotOpenInsertContextMenuRequested(iIndex, iCursorTime, p);
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SlotScrollbarValueChanged()
{
  SetCurrentWindow();
  UpdateTimeSelectionCursor();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SlotSelectionColorChanged()
{
  ForAllLayers([this](CTimelineWidgetLayer* pLayer, qint32 i) {
    auto pBg = dynamic_cast<CTimelineWidgetBackground*>(widget());
    if (nullptr != pLayer && nullptr != pBg)
    {
      if (m_iSelectedIndex == i)
      {
        pLayer->SetHighlight(SelectionColor(), AlternateBackgroundColor());
      }
      else
      {
        pLayer->SetHighlight(QColor(), QColor());
      }
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SlotTimeGridChanged(qint64 iGrid)
{
  ForAllLayers([iGrid](CTimelineWidgetLayer* pLayer, qint32 i) {
    if (nullptr != pLayer)
    {
      pLayer->SetTimeGrid(iGrid);
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SlotZoomChanged(qint32 iZoom)
{
  qint64 iPageStep = timeline::c_iDefaultStepSize*100/iZoom;
  qint64 iMax = nullptr != m_spCurrentSequence ? m_spCurrentSequence->m_iLengthMili : timeline::c_iDefaultLength;
  m_pCustomScrollbar->setMinimum(0);
  m_pCustomScrollbar->setMaximum(std::max(iMax-iPageStep, 0LL));
  // default 100% is page step of 10'000
  m_pCustomScrollbar->setPageStep(iPageStep);
  m_pCustomScrollbar->setSingleStep(iPageStep / 10);

  SetCurrentWindow();
  UpdateTimeSelectionCursor();
}

//----------------------------------------------------------------------------------------
//
QWidget* CTimelineWidget::CreateLayerWidget(const tspSequenceLayer& spLayer) const
{
  auto pWidget =
      new CTimelineWidgetLayer(spLayer, const_cast<CTimelineWidget*>(this), widget(),
                               m_pEditorModel);
  connect(pWidget, &CTimelineWidgetLayer::SignalContentsChanged,
          this, &CTimelineWidget::SignalContentsChanged, Qt::DirectConnection);
  connect(pWidget, &CTimelineWidgetLayer::SignalUserStartedDrag,
          this, &CTimelineWidget::SlotUserStartedDrag, Qt::QueuedConnection);
  connect(pWidget, &CTimelineWidgetLayer::SignalSelected,
          this, &CTimelineWidget::SlotLayerSelected, Qt::QueuedConnection);
  connect(pWidget, &CTimelineWidgetLayer::SignalOpenInsertContextMenuAt,
          this, &CTimelineWidget::SlotOpenInsertContextMenuAt, Qt::QueuedConnection);
  for (QWidget* pW : pWidget->findChildren<QWidget*>())
  {
    pW->installEventFilter(const_cast<CTimelineWidget*>(this));
    pW->setMouseTracking(true);
  }
  pWidget->installEventFilter(const_cast<CTimelineWidget*>(this));
  pWidget->setMouseTracking(true);
  pWidget->SetUndoStack(m_pUndoStack);
  pWidget->SetGridColor(m_pControls->GridColor());
  pWidget->SetOutOfRangeColor(m_pControls->OutOfRangeColor());
  pWidget->SetTimeGrid(m_pControls->TimeGrid());
  return pWidget;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::ForAllLayers(std::function<void(CTimelineWidgetLayer*,qint32)> fn)
{
  if (nullptr != widget())
  {
    QLayout* pLayout = widget()->layout();
    if (nullptr != pLayout)
    {
      for (qint32 i = 0; pLayout->count() > i; ++i)
      {
        if (nullptr != pLayout->itemAt(i))
        {
          if (auto pWidget = dynamic_cast<CTimelineWidgetLayer*>(pLayout->itemAt(i)->widget()))
          {
            fn(pWidget, i);
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
QSize CTimelineWidget::HeadersSize() const
{
  QSize s = m_pControls->minimumSizeHint();
  const_cast<CTimelineWidget*>(this)->ForAllLayers([&s](CTimelineWidgetLayer* pLayer, qint32) {
    QSize s2 = pLayer->HeaderSize();
    s.setWidth(std::max(s.width(), s2.width()));
    s.setHeight(std::max(s.height(), s2.height()));
  });
  return s;
}

//----------------------------------------------------------------------------------------
//
QString CTimelineWidget::OpenInsertContextMenuAt(const QPoint& currentAddPoint, const QPoint& createPoint,
                                                 const QString& sLayerType)
{
  QMenu modelMenu;

  //Add filterbox to the context menu
  auto* pTxtBox = new CSearchWidget(&modelMenu);
  auto* pTxtBoxAction = new QWidgetAction(&modelMenu);
  pTxtBoxAction->setDefaultWidget(pTxtBox);

  modelMenu.addAction(pTxtBoxAction);

  //Add to the context menu
  auto* pListView = new CSequenceEmentList(&modelMenu);
  pListView->SetAllowedCategories(QStringList() << sLayerType);
  auto* pListViewAction = new QWidgetAction(&modelMenu);
  pListViewAction->setDefaultWidget(pListView);
  pListView->Initialize();

  modelMenu.addAction(pListViewAction);

  //Setup filtering
  connect(pTxtBox, &CSearchWidget::SignalFilterChanged, pTxtBox, [&](const QString &text)
          {
            pListView->SetFilter(text);
          });

  QString sRetVal;
  connect(pListView, &CSequenceEmentList::SignalSelectedItem, pListView,
          [&](const QString& sId)
          {
            sRetVal = sId;
            modelMenu.close();
          });

  pTxtBox->setFocus();

  modelMenu.exec(createPoint);
  return sRetVal;
}

//----------------------------------------------------------------------------------------
//
bool CTimelineWidget::IsChildOfLayer(QWidget* pWidget) const
{
  if (nullptr != pWidget)
  {
    QWidget* pParent = pWidget->parentWidget();
    CTimelineWidgetLayer* pLayer = qobject_cast<CTimelineWidgetLayer*>(pWidget);
    while (nullptr == pLayer && nullptr != pParent)
    {
      pLayer = qobject_cast<CTimelineWidgetLayer*>(pParent);
      pParent = pParent->parentWidget();
    }
    return nullptr != pLayer;
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::Resize(QSize newSize)
{
  qint32 iSizeForWidgets = newSize.width();
  qint32 iHeight = height() > widget()->minimumSizeHint().height() ? height() :
                       widget()->minimumSizeHint().height();
  widget()->resize({iSizeForWidgets, iHeight});

  m_pOverlay->move(0, 0);
  m_pOverlay->resize(newSize);

  m_pControls->move(0, 0);
  m_pControls->resize(newSize.width(), m_pControls->minimumSizeHint().height());
  m_pControls->raise();
  m_pControls->repaint();

  m_pCustomScrollbar->resize(width(), horizontalScrollBar()->height());
  m_pCustomScrollbar->move(0, height() - m_pCustomScrollbar->height());
  m_pCustomScrollbar->raise();

  setViewportMargins(0, m_pControls->minimumSizeHint().height(), 0, m_pCustomScrollbar->height());
  SlotZoomChanged(m_pControls->Zoom());
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetCurrentWindow()
{
  m_pControls->SetCurrentWindow(m_pCustomScrollbar->value(), m_pCustomScrollbar->pageStep());
  ForAllLayers([this](CTimelineWidgetLayer* pLayer, qint32) {
    pLayer->SetCurrentWindow(m_pCustomScrollbar->value(), m_pCustomScrollbar->pageStep());
  });
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetSelectedTime()
{
  qint64 iCursorTime = m_pControls->TimeFromCursor();
  if (-1 < iCursorTime)
  {
    m_pControls->SetCurrentTimeStamp(iCursorTime);
    UpdateTimeSelectionCursor();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::UpdateTimeSelectionCursor()
{
  qint32 iCursor = m_pControls->CursorFromCurrentTime();
  if (-1 < iCursor)
  {
    m_pOverlay->SetCurrentTimePosIndicator(iCursor);
  }
  else
  {
    m_pOverlay->SetCurrentTimePosIndicator(-1);
  }
}

#include "TimelineWidget.moc"
