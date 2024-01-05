#include "EditorSequenceEditorWidget.h"

#include "Editor/EditorActionBar.h"
#include "Editor/EditorModel.h"

#include "Editor/SequenceEditor/SequenceEmentList.h"

#include "Widgets/SearchWidget.h"

#include <QMenu>
#include <QWidgetAction>

DECLARE_EDITORWIDGET(CEditorPatternEditorWidget, EEditorWidget::ePatternEditor)

CEditorPatternEditorWidget::CEditorPatternEditorWidget(QWidget* pParent) :
    CEditorWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CEditorSequenceEditorWidget>())
{
  m_spUi->setupUi(this);
}

CEditorPatternEditorWidget::~CEditorPatternEditorWidget()
{
  UnloadProject();
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::Initialize()
{
  m_bInitialized = false;

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::LoadProject(tspProject spProject)
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = spProject;

  //m_spUi->pTopSplitter->setSizes({ width()/4, width() *3/4 });
  m_spUi->pBottomSplitter->setSizes({ height()/2, height()/2 });

  SetLoaded(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = nullptr;

  SetLoaded(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SaveProject()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnHidden()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnShown()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->NewSequence, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotAddNewSequenceButtonClicked);
    disconnect(ActionBar()->m_spUi->AddSequenceLayer, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotAddSequenceLayerButtonClicked);
    disconnect(ActionBar()->m_spUi->RemoveSelectedSequenceLayer, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotRemoveSequenceLayerButtonClicked);
    disconnect(ActionBar()->m_spUi->AddSequenceElement, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotAddSequenceElementButtonClicked);
    disconnect(ActionBar()->m_spUi->RemoveSelectedSequenceElements, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotRemoveSequenceElementsButtonClicked);

    ActionBar()->m_spUi->NewSequence->setEnabled(true);
    ActionBar()->m_spUi->AddSequenceLayer->setEnabled(true);
    ActionBar()->m_spUi->RemoveSelectedSequenceLayer->setEnabled(true);
    ActionBar()->m_spUi->AddSequenceElement->setEnabled(true);
    ActionBar()->m_spUi->RemoveSelectedSequenceElements->setEnabled(true);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnActionBarChanged()
{
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowSequenceEditorActionBar();
    connect(ActionBar()->m_spUi->NewSequence, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotAddNewSequenceButtonClicked);
    connect(ActionBar()->m_spUi->AddSequenceLayer, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotAddSequenceLayerButtonClicked);
    connect(ActionBar()->m_spUi->RemoveSelectedSequenceLayer, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotRemoveSequenceLayerButtonClicked);
    connect(ActionBar()->m_spUi->AddSequenceElement, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotAddSequenceElementButtonClicked);
    connect(ActionBar()->m_spUi->RemoveSelectedSequenceElements, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotRemoveSequenceElementsButtonClicked);

    if (EditorModel()->IsReadOnly())
    {
      ActionBar()->m_spUi->NewSequence->setEnabled(false);
      ActionBar()->m_spUi->AddSequenceLayer->setEnabled(false);
      ActionBar()->m_spUi->RemoveSelectedSequenceLayer->setEnabled(false);
      ActionBar()->m_spUi->AddSequenceElement->setEnabled(false);
      ActionBar()->m_spUi->RemoveSelectedSequenceElements->setEnabled(false);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotAddNewSequenceButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotAddSequenceLayerButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
      if (nullptr == m_spCurrentProject) { return; }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotRemoveSequenceLayerButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
      if (nullptr == m_spCurrentProject) { return; }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotAddSequenceElementButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  OpenContextMenuAt(QPoint(0, 0),
                    m_spUi->pPreviewWidget->parentWidget()->mapToGlobal(
                        m_spUi->pPreviewWidget->pos()));
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotRemoveSequenceElementsButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OpenContextMenuAt(const QPoint& localPoint,
                                                   const QPoint& createPoint)
{
  if (EditorModel()->IsReadOnly()) { return; }

  QMenu modelMenu;

  //Add filterbox to the context menu
  auto* pTxtBox = new CSearchWidget(&modelMenu);
  auto* pTxtBoxAction = new QWidgetAction(&modelMenu);
  pTxtBoxAction->setDefaultWidget(pTxtBox);

  modelMenu.addAction(pTxtBoxAction);

  //Add to the context menu
  auto* pListView = new CSequenceEmentList(&modelMenu);
  auto* pListViewAction = new QWidgetAction(&modelMenu);
  pListViewAction->setDefaultWidget(pListView);
  pListView->Initialize();

  modelMenu.addAction(pListViewAction);

  //Setup filtering
  connect(pTxtBox, &CSearchWidget::SignalFilterChanged, pTxtBox, [&](const QString &text)
          {
            pListView->SetFilter(text);
          });

  connect(pListView, &QTreeView::activated, pListView, [&](QModelIndex idx)
          {
            Q_UNUSED(idx)
            modelMenu.close();
          });

  pTxtBox->setFocus();

  modelMenu.exec(createPoint);
}
