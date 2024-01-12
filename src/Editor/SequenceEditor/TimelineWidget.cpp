#include "TimelineWidget.h"
#include "CommandModifyLayers.h"
#include "TimelineWidgetLayer.h"
#include "TimelineWidgetBackground.h"
#include "ui_TimelineWidget.h"

#include <QResizeEvent>
#include <QScrollBar>
#include <QVBoxLayout>

namespace
{
  const char c_sDefaultName[] = "New Layer";
}

//----------------------------------------------------------------------------------------
//
CTimelineWidget::CTimelineWidget(QWidget* pParent) :
  QScrollArea(pParent),
  m_spUi(std::make_unique<Ui::CTimelineWidget>())
{
  m_spUi->setupUi(this);
  // we will use a custom scrollbar here
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  auto pBg = new CTimelineWidgetBackground(this);
  pBg->setLayout(new QVBoxLayout(pBg));
  setWidget(pBg);
  widget()->layout()->setContentsMargins({0, 0, 0, 0});
}

CTimelineWidget::~CTimelineWidget()
{
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
            }, "Add new "));
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::AddNewElement(const QString& sId)
{
  Q_UNUSED(sId)
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::Clear()
{
  if (nullptr != m_spCurrentSequence)
  {
    assert(false);
    m_spCurrentSequence->m_vspLayers.clear();
  }

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
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::RemoveSelectedLayer()
{
  if (nullptr != m_pUndoStack && nullptr != m_spCurrentSequence)
  {
    if (0 > m_iSelectedIndex &&
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
              }, "Remove "));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetSequence(const tspSequence& spSeq)
{
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
      for (qint32 i = 0; std::max(pLayout->count(),
                                  static_cast<qint32>(m_spCurrentSequence->m_vspLayers.size())); ++i)
      {
        if (m_spCurrentSequence->m_vspLayers.size() > static_cast<size_t>(i))
        {
          auto spLayer = m_spCurrentSequence->m_vspLayers[static_cast<size_t>(i)];
          if (pLayout->count() > i)
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
            pLayout->insertWidget(i, new CTimelineWidgetLayer(spLayer, widget()));
          }
        }
        else if (pLayout->count() > i)
        {
          auto pItem = pLayout->takeAt(i);
          if (nullptr != pItem->widget()) { delete pItem->widget(); }
          if (nullptr != pItem) { delete pItem; }
        }
      }
    }
  }
  else
  {
    Clear();
  }
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
void CTimelineWidget::AddLayerTo(qint32 index, const tspSequenceLayer& spLayer,
                                 tspSequence& spCurrentSequence)
{
  if (nullptr == spCurrentSequence || nullptr == spLayer) { return; }

  if (0 > index || spCurrentSequence->m_vspLayers.size() < index) { return; }

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
    pLayout->insertWidget(index, new CTimelineWidgetLayer(spLayer, widget()));
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::RemoveLayerFrom(qint32 index, tspSequence& spCurrentSequence)
{
  if (nullptr == spCurrentSequence) { return; }

  if (0 > index || spCurrentSequence->m_vspLayers.size() <= index) { return; }

  spCurrentSequence->m_vspLayers.erase(spCurrentSequence->m_vspLayers.begin() + index);

  QVBoxLayout* pLayout = dynamic_cast<QVBoxLayout*>(widget()->layout());
  if (nullptr != pLayout)
  {
    auto pItem = pLayout->takeAt(index);
    if (nullptr != pItem->widget()) { delete pItem->widget(); }
    if (nullptr != pItem) { delete pItem; }
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::paintEvent(QPaintEvent* pEvt)
{
  QScrollArea::paintEvent(pEvt);
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::resizeEvent(QResizeEvent* pEvt)
{
  qint32 iSizeForWidgets = pEvt->size().width();
  if (verticalScrollBar()->isVisible())
  {
    iSizeForWidgets -= verticalScrollBar()->size().width();
  }

  for (qint32 i = 0; widget()->layout()->count() > i; ++i)
  {
    auto pItem = widget()->layout()->itemAt(i);
    if (nullptr != pItem->widget())
    {
      pItem->widget()->resize({iSizeForWidgets, pItem->widget()->height()});
    }
  }
}
