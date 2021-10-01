#include "EditorLayoutCompact.h"
#include "Application.h"
#include "ui_EditorLayoutCompact.h"
#include "Editor/EditorActionBar.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorWidgetTypes.h"
#include "Editor/EditorWidgets/EditorResourceDisplayWidget.h"
//#include "Editor/Tutorial/MainScreenTutorialStateSwitchHandler.h"
#include "Systems/HelpFactory.h"
#include "Widgets/HelpOverlay.h"
#include "Widgets/ShortcutButton.h"
#include <QAction>

namespace
{
  const std::map<EEditorWidget, QString> m_sEditorKeyBindingMap =
  {
    { EEditorWidget::eResourceWidget, "Resource" },
    { EEditorWidget::eResourceDisplay, "MediaPlayer" },
    { EEditorWidget::eProjectSettings, "Settings" },
    { EEditorWidget::eSceneNodeWidget, "Nodes" },
    { EEditorWidget::eSceneCodeEditorWidget, "Code" }
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
  m_iCurrentView(EEditorWidget::eProjectSettings)
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

  ChangeView(m_iCurrentView);
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
void CEditorLayoutCompact::InitializeImpl()
{
  // state switch handler
  //m_spStateSwitchHandler =
  //    std::make_shared<CMainScreenTutorialStateSwitchHandler>(this, GetTutorialOverlay());
  //EditorModel()->AddTutorialStateSwitchHandler(m_spStateSwitchHandler);

  m_spUi->pActionBar->SetActionBarPosition(CEditorActionBar::eLeft);
  m_spUi->pActionBar->Initialize();

  // create all the buttons to switch views
  QLayout* pLayout = m_spUi->pViewsButtonContainer->layout();
  pLayout->addItem(new QSpacerItem(0, 20));
  VisitWidgets([this, pLayout](QPointer<CEditorWidgetBase> pWidget, EEditorWidget type) {
    Q_UNUSED(pWidget)

    // Key-bindings will be set later
    if (EEditorWidget::eResourceDisplay != type._to_integral())
    {
      CShortcutButton* pButton = new CShortcutButton(m_spUi->pViewsButtonContainer);
      pButton->setObjectName(type._to_string());
      pButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
      pButton->setProperty(c_sEditorProperty, type._to_integral());
      auto it = m_sEditorKeyBindingMap.find(type);
      if (m_sEditorKeyBindingMap.end() != it)
      {
        pButton->SetShortcut(CApplication::Instance()->Settings()->keyBinding(it->second));
      }
      connect(pButton, &QPushButton::clicked, this, &CEditorLayoutCompact::SlotViewSwitchButtonClicked);
      pLayout->addWidget(pButton);
    }
  });
  pLayout->addItem(new QSpacerItem(0, 20));

  // help
  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pViewsButtonContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sViewSelectorHelpId);
    wpHelpFactory->RegisterHelp(c_sViewSelectorHelpId, ":/resources/help/editor/selection_combobox_help.html");
  }
}

