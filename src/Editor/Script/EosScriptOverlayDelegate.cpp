#include "EosScriptOverlayDelegate.h"
#include "EosCommandWidgets.h"
#include "EosScriptModelItem.h"

#include "Editor/Resources/ResourceTreeItemModel.h"

#include <QScrollBar>
#include <QTreeView>
#include <QVBoxLayout>

CEosScriptOverlayDelegate::CEosScriptOverlayDelegate(QWidget* pParent) :
  COverlayBase(0, pParent)
{
  setObjectName("pEosScriptOverlayDelegate");
  QLayout* pLayout = new QVBoxLayout(this);
  setLayout(pLayout);
}
CEosScriptOverlayDelegate::~CEosScriptOverlayDelegate()
{
}

//----------------------------------------------------------------------------------------
//
SItemIndexPath CEosScriptOverlayDelegate::CurrentPath() const
{
  return m_path;
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CEosScriptOverlayDelegate::CurrentProperties() const
{
  return m_propsCopy;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptOverlayDelegate::Initialize(CResourceTreeItemModel* pEditorModel)
{
  m_pEditorModel = pEditorModel;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptOverlayDelegate::Show(const SItemIndexPath& path,
                                     const QString& sType, const tInstructionMapValue& props)
{
  CEosCommandWidgetBase* pWidget = nullptr;
  bool bNewWidget = false;

  if (path != m_path)
  {
    bNewWidget = true;
    ClearLayout();
    m_path = path;
    m_propsCopy = props;

    pWidget = eos::CreateWidgetFromType(sType, this);
  }
  else
  {
    m_propsCopy = props;
    if (auto pItem = layout()->itemAt(0))
    {
      pWidget = dynamic_cast<CEosCommandWidgetBase*>(pItem->widget());
    }
  }

  if (nullptr != pWidget && pWidget->HasUi())
  {
    pWidget->SetProperties(&m_propsCopy);

    if (bNewWidget)
    {
      pWidget->SetResourceModel(m_pEditorModel);
      pWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
      layout()->addWidget(pWidget);
      layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

      connect(pWidget, &CEosCommandWidgetBase::SignalChangedProperties, this,
              &CEosScriptOverlayDelegate::SignalCurrentItemChanged);
      connect(pWidget, &CEosCommandWidgetBase::SignalInvalidateItemChildren, this,
              &CEosScriptOverlayDelegate::SignalInvalidateItemChildren);
    }

    COverlayBase::Show();
  }
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptOverlayDelegate::IsForcedOpen() const
{
  QLayoutItem* pItem = layout()->itemAt(0);
  if (nullptr != pItem && nullptr != pItem->widget())
  {
    if (auto pWidget = dynamic_cast<CEosCommandWidgetBase*>(pItem->widget()))
    {
      return pWidget->IsForcedOpen();
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptOverlayDelegate::Climb()
{
  ClimbToFirstInstanceOf("CEosScriptEditorView", false);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptOverlayDelegate::Hide()
{
  ClearLayout();
  m_propsCopy = tInstructionMapValue();
  m_path = SItemIndexPath();
  COverlayBase::Hide();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptOverlayDelegate::Resize()
{
  qint32 iOffset = 0;
  QTreeView* pTree = dynamic_cast<QTreeView*>(m_pTargetWidget.data());
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
void CEosScriptOverlayDelegate::ClearLayout()
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
