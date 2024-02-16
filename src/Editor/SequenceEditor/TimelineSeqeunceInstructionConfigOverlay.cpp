#include "TimelineSeqeunceInstructionConfigOverlay.h"
#include "TimelineSeqeunceInstructionWidgets.h"
#include "TimelineWidgetLayer.h"

#include "Editor/Resources/ResourceTreeItemModel.h"

#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

CTimelineSeqeunceInstructionConfigOverlay::CTimelineSeqeunceInstructionConfigOverlay(
    CTimelineWidgetLayer* pParent) :
  COverlayBase{0, pParent},
  m_pParent(pParent)
{
  setObjectName("pTimelineSeqeunceInstructionConfigOverlay");
  QLayout* pLayout = new QVBoxLayout(this);
  setLayout(pLayout);
}

CTimelineSeqeunceInstructionConfigOverlay::~CTimelineSeqeunceInstructionConfigOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionConfigOverlay::Show(qint64 iInstr,
                                                     const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  CTimelineSeqeunceInstructionWidgetBase* pWidget = nullptr;
  bool bNewWidget = false;

  if (iInstr != m_iInstrId)
  {
    bNewWidget = true;
    ClearLayout();
    m_iInstrId = iInstr;
    m_spInstr = spInstr->Clone();

    pWidget = sequence::CreateWidgetFromInstruction(spInstr, this);
  }
  else
  {
    m_spInstr = spInstr->Clone();
    if (auto pItem = layout()->itemAt(0))
    {
      pWidget = dynamic_cast<CTimelineSeqeunceInstructionWidgetBase*>(pItem->widget());
    }
  }

  if (nullptr != pWidget && pWidget->HasUi())
  {
    pWidget->SetProperties(m_spInstr);

    if (bNewWidget)
    {
      pWidget->SetResourceModel(m_pParent->ResourceItemModel());
      pWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
      layout()->addWidget(pWidget);
      layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

      connect(pWidget, &CTimelineSeqeunceInstructionWidgetBase::SignalChangedProperties, this,
              [this, pWidget]() {
        m_spInstr = pWidget->Properties();
        emit SignalCurrentItemChanged();
      });
    }

    COverlayBase::Show();
  }
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionConfigOverlay::CurrentInstructionParameters() const
{
  return m_spInstr;
}

//----------------------------------------------------------------------------------------
//
bool CTimelineSeqeunceInstructionConfigOverlay::IsForcedOpen() const
{
  QLayoutItem* pItem = layout()->itemAt(0);
  if (nullptr != pItem && nullptr != pItem->widget())
  {
    if (auto pWidget = dynamic_cast<CTimelineSeqeunceInstructionWidgetBase*>(pItem->widget()))
    {
      return pWidget->IsForcedOpen();
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionConfigOverlay::Climb()
{
  ClimbToFirstInstanceOf("CTimelineWidget", false);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionConfigOverlay::Hide()
{
  ClearLayout();
  m_spInstr = nullptr;
  m_iInstrId = -1;
  COverlayBase::Hide();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionConfigOverlay::Resize()
{
  qint32 iOffset = 0;
  QScrollArea* pTree = dynamic_cast<QScrollArea*>(m_pTargetWidget.data());
  if (nullptr != pTree)
  {
    if (m_pTargetWidget->width() > m_pTargetWidget->height())
    {
      iOffset = pTree->verticalScrollBar()->isVisible() ? pTree->verticalScrollBar()->width() : 0;
    }
    else
    {
      iOffset = pTree->horizontalScrollBar()->isVisible() ? pTree->horizontalScrollBar()->height() : 0;
    }
  }

  if (m_pTargetWidget->width() > m_pTargetWidget->height())
  {
    resize(m_pTargetWidget->width() / 2 - iOffset, m_pTargetWidget->height());
    move(m_pTargetWidget->width() / 2, 0);
  }
  else
  {
    resize(m_pTargetWidget->width(), m_pTargetWidget->height() / 2 - iOffset);
    move(0, m_pTargetWidget->height() / 2);
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionConfigOverlay::ClearLayout()
{
  while (layout()->count() > 0)
  {
    auto pItem = layout()->takeAt(0);
    if (nullptr != pItem)
    {
      if (nullptr != pItem->widget()) { pItem->widget()->deleteLater(); }
      delete pItem;
    }
  }
}
