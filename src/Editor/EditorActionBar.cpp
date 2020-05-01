#include "EditorActionBar.h"
#include "Application.h"
#include "EditorWidgetTypes.h"
#include "Settings.h"
#include "ui_EditorActionBar.h"
#include "Systems/HelpFactory.h"
#include "Widgets/HelpOverlay.h"

namespace {
  const QString c_sRenameProjectHelpId =  "Editor/RenameProject";
  const QString c_sSaveProjectHelpId =    "Editor/SaveProject";
  const QString c_sExitProjectHelpId =    "Editor/ExitProject";

  const QString c_sAddResourceHelpId =    "Editor/AddResource";
  const QString c_sAddWebResourceHelpId = "Editor/AddWebResource";
  const QString c_sRemoveResourceHelpId = "Editor/RemoveResource";
  const QString c_sTitleCardHelpId =      "Editor/TitleCard";
  const QString c_sMapHelpId =            "Editor/Map";

  const QString c_sPlayHelpId =           "Editor/Play";
  const QString c_sPauseHelpId =          "Editor/Pause";
  const QString c_sStopHelpId =           "Editor/Stop";

  const QString c_sAddNodeHelpId =        "Editor/AddNode";
  const QString c_sRemoveNodeHelpId =     "Editor/RemoveNode";

  const QString c_sStartDebugHelpId =     "Editor/StartDebugCode";
  const QString c_sStopDebugHelpId =      "Editor/StopDebugCode";
  const QString c_sAddShowImageHelpId =   "Editor/AddShowImageCode";
  const QString c_sAddShowIconHelpId =    "Editor/AddShowIconCode";
  const QString c_sAddShowBGHelpId =      "Editor/AddShowBGCode";
  const QString c_sAddTextHelpId =        "Editor/AddTextCode";
  const QString c_sAddTimerHelpId =       "Editor/AddTimerCode";
  const QString c_sAddThreadHelpId =      "Editor/AddThreadCode";

  const QString c_sAddFetishHelpId =      "Editor/AddFetish";
  const QString c_sRemoveFetishHelpId =   "Editor/RemoveFetish";
}

CEditorActionBar::CEditorActionBar(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEditorActionBar>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_position(eNone),
  m_iCurrentDisplayType(-1)
{
  m_spUi->setupUi(this);
}

CEditorActionBar::~CEditorActionBar()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::SetActionBarPosition(EActionBarPosition position)
{
  if (!IsInitialized())
  {
    m_position = position;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::Initialize()
{
  SetInitialized(false);

  HideAllBars();
  SlotKeyBindingsChanged();

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pTitleLineEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRenameProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sRenameProjectHelpId, ":/resources/help/editor/project_name_help.html");
    m_spUi->SaveButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSaveProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sSaveProjectHelpId, ":/resources/help/editor/saveproject_button_help.html");
    m_spUi->ExitButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sExitProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sExitProjectHelpId, ":/resources/help/editor/exitproject_button_help.html");

    m_spUi->AddResourceButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddResourceHelpId);
    wpHelpFactory->RegisterHelp(c_sAddResourceHelpId, ":/resources/help/editor/addresource_button_help.html");
    m_spUi->AddWebResourceButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddWebResourceHelpId);
    wpHelpFactory->RegisterHelp(c_sAddWebResourceHelpId, ":/resources/help/editor/addwebresource_button_help.html");
    m_spUi->RemoveResourceButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRemoveResourceHelpId);
    wpHelpFactory->RegisterHelp(c_sRemoveResourceHelpId, ":/resources/help/editor/removeresource_button_help.html");
    m_spUi->TitleCardButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sTitleCardHelpId);
    wpHelpFactory->RegisterHelp(c_sTitleCardHelpId, ":/resources/help/editor/titlecard_button_help.html");
    m_spUi->MapButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sMapHelpId);
    wpHelpFactory->RegisterHelp(c_sMapHelpId, ":/resources/help/editor/map_button_help.html");

    m_spUi->PlayButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sPlayHelpId);
    wpHelpFactory->RegisterHelp(c_sPlayHelpId, ":/resources/help/editor/play_button_help.html");
    m_spUi->PauseButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sPauseHelpId);
    wpHelpFactory->RegisterHelp(c_sPauseHelpId, ":/resources/help/editor/pause_button_help.html");
    m_spUi->StopButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sStopHelpId);
    wpHelpFactory->RegisterHelp(c_sStopHelpId, ":/resources/help/editor/stop_button_help.html");

    m_spUi->AddNodeButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddNodeHelpId);
    wpHelpFactory->RegisterHelp(c_sAddNodeHelpId, ":/resources/help/editor/addnode_button_help.html");
    m_spUi->RemoveNodeButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRemoveNodeHelpId);
    wpHelpFactory->RegisterHelp(c_sRemoveNodeHelpId, ":/resources/help/editor/removenode_button_help.html");

    m_spUi->DebugButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sStartDebugHelpId);
    wpHelpFactory->RegisterHelp(c_sStartDebugHelpId, ":/resources/help/editor/startdebug_button_help.html");
    m_spUi->StopDebugButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sStopDebugHelpId);
    wpHelpFactory->RegisterHelp(c_sStopDebugHelpId, ":/resources/help/editor/stopdebug_button_help.html");
    m_spUi->AddShowImageCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddShowImageHelpId);
    wpHelpFactory->RegisterHelp(c_sAddShowImageHelpId, ":/resources/help/editor/addshowimage_button_help.html");
    m_spUi->AddShowIconCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddShowIconHelpId);
    wpHelpFactory->RegisterHelp(c_sAddShowIconHelpId, ":/resources/help/editor/addshowicon_button_help.html");
    m_spUi->AddShowBackgroundCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddShowBGHelpId);
    wpHelpFactory->RegisterHelp(c_sAddShowBGHelpId, ":/resources/help/editor/addshowbg_button_help.html");
    m_spUi->AddTextCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddTextHelpId);
    wpHelpFactory->RegisterHelp(c_sAddTextHelpId, ":/resources/help/editor/addtext_button_help.html");
    m_spUi->AddTimerCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddTimerHelpId);
    wpHelpFactory->RegisterHelp(c_sAddTimerHelpId, ":/resources/help/editor/addtimer_button_help.html");
    m_spUi->AddThreadCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddThreadHelpId);
    wpHelpFactory->RegisterHelp(c_sAddThreadHelpId, ":/resources/help/editor/addthread_button_help.html");

    m_spUi->AddNodeButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddFetishHelpId);
    wpHelpFactory->RegisterHelp(c_sAddFetishHelpId, ":/resources/help/editor/addfetish_button_help.html");
    m_spUi->RemoveNodeButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRemoveFetishHelpId);
    wpHelpFactory->RegisterHelp(c_sRemoveFetishHelpId, ":/resources/help/editor/removefetish_button_help.html");
  }

  connect(m_spSettings.get(), &CSettings::keyBindingsChanged,
          this, &CEditorActionBar::SlotKeyBindingsChanged);

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::HideAllBars()
{
  m_spUi->pSceneNodeEditorContainer->hide();
  m_spUi->pMediaPlayerActionBar->hide();
  m_spUi->pProjectContainer->hide();
  m_spUi->pResourcesContainer->hide();
  m_spUi->pCodeEditorContainer->hide();
  m_spUi->pProjectSettingsEditorContainer->hide();

  m_iCurrentDisplayType = -1;
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::ShowCodeActionBar()
{
  HideAllBars();
  m_spUi->pCodeEditorContainer->show();
  m_spUi->DebugButton->show();
  m_spUi->StopDebugButton->hide();

  m_iCurrentDisplayType = EEditorWidget::eSceneCodeEditorWidget;
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::ShowNodeEditorActionBar()
{
  HideAllBars();
  m_spUi->pSceneNodeEditorContainer->show();

  m_iCurrentDisplayType = EEditorWidget::eSceneNodeWidget;
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::ShowMediaPlayerActionBar()
{
  HideAllBars();
  m_spUi->pMediaPlayerActionBar->show();

  m_iCurrentDisplayType = EEditorWidget::eResourceDisplay;
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::ShowProjectActionBar()
{
  HideAllBars();
  m_spUi->pProjectContainer->show();

  m_iCurrentDisplayType = -1;
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::ShowProjectSettingsActionBar()
{
  HideAllBars();
  m_spUi->pProjectSettingsEditorContainer->show();

  m_iCurrentDisplayType = EEditorWidget::eProjectSettings;
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::ShowResourceActionBar()
{
  HideAllBars();
  m_spUi->pResourcesContainer->show();

  m_iCurrentDisplayType = EEditorWidget::eResourceWidget;
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::SlotKeyBindingsChanged()
{
  // set bindings
  QString sKey;
  switch (m_position)
  {
    case eLeft:
    {
      sKey = QString("Left_%1");
      break;
    }
    case eRight:
    {
      sKey = QString("Right_%1");
      break;
    }
  default: break;
  }

  // connect actions
  if (m_position == eTop)
  {
    m_spUi->ExitButton->SetShortcut(m_spSettings->keyBinding("Exit"));
    m_spUi->HelpButton->SetShortcut(m_spSettings->keyBinding("Help"));
    m_spUi->SaveButton->SetShortcut(m_spSettings->keyBinding("Save"));
  }
  else
  {
    m_spUi->AddResourceButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->AddWebResourceButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));
    m_spUi->RemoveResourceButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(3)));
    m_spUi->TitleCardButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(4)));
    m_spUi->MapButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(5)));

    m_spUi->PlayButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->PauseButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));
    m_spUi->StopButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(3)));

    m_spUi->AddFetishButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->RemoveFetishButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));

    m_spUi->AddNodeButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->RemoveNodeButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));

    m_spUi->DebugButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->StopDebugButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->AddShowImageCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));
    m_spUi->AddShowIconCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(3)));
    m_spUi->AddShowBackgroundCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(4)));
    m_spUi->AddTextCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(5)));
    m_spUi->AddTimerCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(6)));
    m_spUi->AddThreadCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(7)));
  }
}
