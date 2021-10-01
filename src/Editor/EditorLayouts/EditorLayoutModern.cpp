#include "EditorLayoutModern.h"
#include "ui_EditorLayoutModern.h"
#include "Editor/EditorActionBar.h"
#include "Editor/EditorWidgets/EditorWidgetBase.h"
#include <QComboBox>

CEditorLayoutModern::CEditorLayoutModern(QWidget *parent) :
  CEditorLayoutBase(parent),
  m_spUi(std::make_unique<Ui::CEditorLayoutModern>())
{
  m_spUi->setupUi(this);
}

CEditorLayoutModern::~CEditorLayoutModern()
{
  QLayout* pLayout = m_spUi->pBottom->layout();
  if (nullptr != pLayout)
  {
    while(auto item = pLayout->takeAt(0))
    {
      CEditorWidgetBase* pEditor = qobject_cast<CEditorWidgetBase*>(item->widget());
      pEditor->TakeFromLayout();
      pEditor->OnHidden();
      pEditor->setVisible(false);
      delete item;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutModern::ProjectLoaded(tspProject spCurrentProject, bool bModified)
{
  m_spUi->pTop->ProjectLoaded(spCurrentProject, bModified);

  qint32 iIndex = m_spUi->pTop->LeftComboBox()->findData(EEditorWidget::eResourceDisplay);
  m_spUi->pTop->LeftComboBox()->setCurrentIndex(iIndex);

  iIndex = m_spUi->pTop->LeftComboBox()->findData(EEditorWidget::eResourceWidget);
  m_spUi->pTop->LeftComboBox()->removeItem(iIndex);
  iIndex = m_spUi->pTop->RightComboBox()->findData(EEditorWidget::eResourceWidget);
  m_spUi->pTop->RightComboBox()->removeItem(iIndex);

  QLayout* pLayout = m_spUi->pBottom->layout();
  if (nullptr != pLayout)
  {
    QPointer<CEditorWidgetBase> pWidget = GetWidget(EEditorWidget::eResourceWidget);
    pWidget->setVisible(true);
    pLayout->addWidget(pWidget);
    pWidget->OnShown();
    pWidget->SetActionBar(m_spUi->pActionBarLeft);
  }

  m_spUi->pSplitter->setSizes({height() *3/4, height()/4});
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutModern::ProjectUnloaded()
{
  m_spUi->pTop->ProjectUnloaded();
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutModern::InitializeImpl()
{
  m_spUi->pTop->Initialize(m_pLayoutViewProvider, EditorModel());

  m_spUi->pActionBarLeft->SetActionBarPosition(CEditorActionBar::eLeft);
  m_spUi->pActionBarLeft->Initialize();
}
