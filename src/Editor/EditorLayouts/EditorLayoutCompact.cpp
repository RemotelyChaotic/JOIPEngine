#include "EditorLayoutCompact.h"
#include "Application.h"
#include "ui_EditorLayoutCompact.h"
#include "Editor/EditorActionBar.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorWidgetTypes.h"
#include "Editor/EditorWidgets/EditorResourceDisplayWidget.h"
#include "Editor/Tutorial/CompactTutorialStateSwitchHandler.h"
#include "Systems/HelpFactory.h"
#include "Widgets/HelpOverlay.h"
#include "Widgets/ShortcutButton.h"
#include <QAction>

DECLARE_EDITORLAYOUT(CEditorLayoutCompact, CSettings::eCompact)

namespace
{
  const std::map<EEditorWidget, QString> m_sEditorKeyBindingMap =
  {
    { EEditorWidget::eResourceWidget, "LeftTab_Resource" },
    { EEditorWidget::eResourceDisplay, "LeftTab_MediaPlayer" },
    { EEditorWidget::eProjectSettings, "LeftTab_Settings" },
    { EEditorWidget::eSceneNodeWidget, "LeftTab_Nodes" },
    { EEditorWidget::eSceneCodeEditorWidget, "LeftTab_Code" },
    { EEditorWidget::ePatternEditor, "LeftTab_Sequence" },
    { EEditorWidget::eDialogEditor, "LeftTab_Dialog" }
  };
  const std::map<EEditorWidget, QString> m_sEditorToolTipMap =
  {
    { EEditorWidget::eResourceWidget, "Switch to panel for resource management" },
    { EEditorWidget::eResourceDisplay, "Switch to panel for resource and undo stack display" },
    { EEditorWidget::eProjectSettings, "Switch to panel for Project settings" },
    { EEditorWidget::eSceneNodeWidget, "Switch to panel for scene node configuration" },
    { EEditorWidget::eSceneCodeEditorWidget, "Switch to panel for editing scene code" },
    { EEditorWidget::ePatternEditor, "Switch to panel for editing sequences" },
    { EEditorWidget::eDialogEditor, "Switch to panel for editing dialogs" }
  };

  const char*   c_sEditorProperty = "Editor";
  const QString c_sViewSelectorHelpId =  "Editor/ViewSelector";
}

//----------------------------------------------------------------------------------------
//
CEditorLayoutCompact::CEditorLayoutCompact(QWidget *parent) :
  CEditorLayoutBase(parent),
  m_spUi(std::make_unique<Ui::CEditorLayoutCompact>()),
  m_spCurrentProject(nullptr),
  m_vpKeyBindingActions(),
  // don't set to projectSettings, so we can set it on initialization
  m_iCurrentView(EEditorWidget::_from_integral(EEditorWidget(EEditorWidget::eProjectSettings)._to_integral()+1))
{
  m_spUi->setupUi(this);
}

CEditorLayoutCompact::~CEditorLayoutCompact()
{
  m_vpKeyBindingActions.clear();
  ChangeView(-1);
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutCompact::ProjectLoaded(tspProject spCurrentProject, bool bModified)
{
  Q_UNUSED(bModified)

  m_spCurrentProject = spCurrentProject;

  ChangeView(EEditorWidget::eProjectSettings);
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutCompact::ProjectUnloaded()
{
  m_spCurrentProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutCompact::ChangeView(qint32 iView)
{
  if (m_iCurrentView._to_integral() == iView) { return; }

  QLayout* pLayout = m_spUi->pMainWidget->layout();
  while (auto item = pLayout->takeAt(0))
  {
    CEditorWidgetBase* pEditor = qobject_cast<CEditorWidgetBase*>(item->widget());
    pEditor->TakeFromLayout();
    pEditor->OnHidden();
    pEditor->setVisible(false);
    delete item;
  }

  if (-1 < iView && EEditorWidget::_size_constant > static_cast<size_t>(iView))
  {
    EEditorWidget iEnumValue = EEditorWidget::_from_integral(iView);
    QPointer<CEditorWidgetBase> pWidget = GetWidget(iEnumValue);
    if (nullptr != pWidget)
    {
      pWidget->setVisible(true);
      pLayout->addWidget(pWidget);
      pWidget->OnShown();
      pWidget->SetActionBar(m_spUi->pActionBar);
    }

    m_iCurrentView = iEnumValue;
    emit SignalViewChanged(m_iCurrentView);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutCompact::SetButtonVisible(qint32 iView, bool bVisible)
{
  QLayout* pLayout = m_spUi->pViewsButtonContainer->layout();
  if (nullptr != pLayout)
  {
    for (qint32 i = 0; pLayout->count() > i; ++i)
    {
      QLayoutItem* pItem = pLayout->itemAt(i);
      if (nullptr != pItem && nullptr != pItem->widget() &&
          pItem->widget()->property(c_sEditorProperty).toInt() == iView)
      {
        pItem->widget()->setVisible(bVisible);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutCompact::SlotViewSwitchButtonClicked()
{
  ChangeView(sender()->property(c_sEditorProperty).toInt());
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutCompact::InitializeImpl(bool bWithTutorial)
{
  // state switch handler
  if (bWithTutorial)
  {
    m_spStateSwitchHandler =
        std::make_shared<CCompactTutorialStateSwitchHandler>(this, GetTutorialOverlay());
    EditorModel()->AddTutorialStateSwitchHandler(m_spStateSwitchHandler);
  }

  m_spUi->pActionBar->SetActionBarPosition(CEditorActionBar::eLeft);
  m_spUi->pActionBar->Initialize();

  // create all the buttons to switch views
  QLayout* pLayout = m_spUi->pViewsButtonContainer->layout();
  pLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));
  VisitWidgets([this, pLayout](QPointer<CEditorWidgetBase> pWidget, EEditorWidget type) {
    Q_UNUSED(pWidget)

    // Key-bindings will be set later
    CShortcutButton* pButton = new CShortcutButton(m_spUi->pViewsButtonContainer);
    pButton->setObjectName(type._to_string());
    pButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    pButton->setProperty(c_sEditorProperty, type._to_integral());
    auto it = m_sEditorToolTipMap.find(type);
    if (m_sEditorToolTipMap.end() != it)
    {
      pButton->setToolTip(it->second);
    }
    it = m_sEditorKeyBindingMap.find(type);
    if (m_sEditorKeyBindingMap.end() != it)
    {
      pButton->SetShortcut(CApplication::Instance()->Settings()->keyBinding(it->second));
    }
    connect(pButton, &QPushButton::clicked, this, &CEditorLayoutCompact::SlotViewSwitchButtonClicked);
    pLayout->addWidget(pButton);
  });
  pLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));

  // help
  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pViewsButtonContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sViewSelectorHelpId);
    wpHelpFactory->RegisterHelp(c_sViewSelectorHelpId, ":/resources/help/editor/selection_combobox_help.html");
  }
}

