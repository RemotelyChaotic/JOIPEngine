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
  const QString c_sUndoHelpId =           "Editor/Undo";
  const QString c_sRedoHelpId =           "Editor/Redo";
  const QString c_sExportProjectHelpId =  "Editor/ExportProject";
  const QString c_sToolsHelpId =          "Editor/Tools";
  const QString c_sExitProjectHelpId =    "Editor/ExitProject";

  const QString c_sChangeViewHelpId =     "Editor/ChangeResourceView";
  const QString c_sCdUpHelpId =           "Editor/CdUp";
  const QString c_sAddResourceHelpId =    "Editor/AddResource";
  const QString c_sAddWebResourceHelpId = "Editor/AddWebResource";
  const QString c_sRemoveResourceHelpId = "Editor/RemoveResource";
  const QString c_sImageSourceHelpId =    "Editor/ImageSource";
  const QString c_sTitleCardHelpId =      "Editor/TitleCard";
  const QString c_sTagsHelpId =           "Editor/ResourceTags";
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
  const QString c_sAddMetronomeHelpId =   "Editor/AddMetronomeCode";
  const QString c_sAddTimerHelpId =       "Editor/AddTimerCode";
  const QString c_sAddThreadHelpId =      "Editor/AddThreadCode";
  const QString c_sAddNotificationHelpId ="Editor/AddNotificationCode";
  const QString c_sAddDeviceHelpId =      "Editor/AddDeviceCode";
  const QString c_sAddEosImageHelpId =    "Editor/EOS/AddEosImageInstruction";
  const QString c_sAddEosAudioHelpId =    "Editor/EOS/AddEosAudioInstruction";
  const QString c_sAddEosSayHelpId =      "Editor/EOS/AddEosSayInstruction";
  const QString c_sAddEosPromptHelpId =   "Editor/EOS/AddEosAddPromptInstruction";
  const QString c_sAddEosTimerHelpId =    "Editor/EOS/AddEosAddTimerInstruction";
  const QString c_sAddEosChioceHelpId =   "Editor/EOS/AddEosChoiceInstruction";
  const QString c_sAddEosIfHelpId =       "Editor/EOS/AddEosIfInstruction";
  const QString c_sAddEosNotCreateHelpId ="Editor/EOS/AddEosNotificationCreateInstruction";
  const QString c_sAddEosNotCloseHelpId = "Editor/EOS/AddEosNotificationCloseInstruction";
  const QString c_sAddEosEvalHelpId =     "Editor/EOS/AddEosEvalInstruction";
  const QString c_sAddEosGotoHelpId =     "Editor/EOS/AddEosGotoInstruction";
  const QString c_sAddEosEndHelpId =      "Editor/EOS/AddEosEndInstruction";
  const QString c_sAddEosEnableHelpId =   "Editor/EOS/AddEosEnableInstruction";
  const QString c_sAddEosDisableHelpId =  "Editor/EOS/AddEosDisableInstruction";
  const QString c_sAddEosRemoveHelpId =   "Editor/EOS/AddEosRemoveInstruction";

  const QString c_sAddFetishHelpId =      "Editor/AddFetish";
  const QString c_sRemoveFetishHelpId =   "Editor/RemoveFetish";

  const QString c_sDebugSeqHelpId =        "Editor/DebugSequence";
  const QString c_sAddSeqHelpId =          "Editor/CreateNewSequence";
  const QString c_sEditSeqHelpId =         "Editor/EditSequenceProperties";
  const QString c_sAddSeqElemHelpId =      "Editor/AddSequenceElement";
  const QString c_sRemoveSeqElemsHelpId =  "Editor/RemoveSelectedSequenceElements";
  const QString c_sAddSeqLayerHelpId =     "Editor/AddSequenceLayer";
  const QString c_sRemoveSeqLayerHelpId =  "Editor/RemoveSelectedSequenceLayers";

  const QString c_sAddDialogueHelpId =        "Editor/AddDialogue";
  const QString c_sAddCondDialogueHelpId =    "Editor/AddConditionalDialogue";
  const QString c_sRemoveDialogueHelpId =     "Editor/RemoveDialogue";
  const QString c_sAddDialogueCategoryHelpId ="Editor/AddDialogueCategory";
  const QString c_sRemoveDialogueCategoryHelpId ="Editor/RemoveDialogueCategory";
  const QString c_sEditDialogueHelpId =       "Editor/EditDialogue";
  const QString c_sEditDialogueTagsHelpId =   "Editor/EditDialogueTags";
}

CEditorActionBar::CEditorActionBar(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEditorActionBar>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_position(eNone),
  m_iCurrentDisplayType(-1)
{
  m_spUi->setupUi(this);
  m_spUi->pCodeEditorContainer->setFixedWidth(m_spUi->pCodeEditorContainerStack->sizeHint().width());
}

CEditorActionBar::~CEditorActionBar()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CEditorActionBar::Spacing() const
{
  return m_iSpacing;
}

//----------------------------------------------------------------------------------------
//
qint32 CEditorActionBar::CurrentActionBar()
{
  return m_iCurrentDisplayType;
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::SetSpacing(qint32 iValue)
{
  m_iSpacing = iValue;

  auto fnSetSpacing = [&iValue](QLayout* pLayout) {
    if (nullptr != pLayout)
    {
      pLayout->setSpacing(iValue);
    }
  };

  fnSetSpacing(m_spUi->pSceneNodeEditorContainer->layout());
  fnSetSpacing(m_spUi->pMediaPlayerActionBar->layout());
  fnSetSpacing(m_spUi->pProjectContainer->layout());
  fnSetSpacing(m_spUi->pResourcesContainer->layout());
  fnSetSpacing(m_spUi->pCodeEditorContainer->layout());
  fnSetSpacing(m_spUi->pProjectSettingsEditorContainer->layout());
}

//----------------------------------------------------------------------------------------
//
CEditorActionBar::EActionBarPosition CEditorActionBar::ActionBarPosition() const
{
  return m_position;
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

  // Unused, so don't show
  m_spUi->MapButton->hide();

  HideAllBars();
  SlotKeyBindingsChanged();

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pTitleLineEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRenameProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sRenameProjectHelpId, ":/resources/help/editor/project_name_help.html");
    m_spUi->SaveButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSaveProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sSaveProjectHelpId, ":/resources/help/editor/saveproject_button_help.html");
    m_spUi->UndoButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sUndoHelpId);
    wpHelpFactory->RegisterHelp(c_sUndoHelpId, ":/resources/help/editor/undo_button_help.html");
    m_spUi->RedoButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRedoHelpId);
    wpHelpFactory->RegisterHelp(c_sRedoHelpId, ":/resources/help/editor/redo_button_help.html");
    m_spUi->ExportButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sExportProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sExportProjectHelpId, ":/resources/help/editor/exportproject_button_help.html");
    m_spUi->ToolsButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sToolsHelpId);
    wpHelpFactory->RegisterHelp(c_sToolsHelpId, ":/resources/help/editor/tools_button_help.html");
    m_spUi->ExitButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sExitProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sExitProjectHelpId, ":/resources/help/editor/exitproject_button_help.html");

    m_spUi->AddResourceButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sChangeViewHelpId);
    wpHelpFactory->RegisterHelp(c_sChangeViewHelpId, ":/resources/help/editor/changetotreefolderview_button_help.html");
    m_spUi->AddWebResourceButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCdUpHelpId);
    wpHelpFactory->RegisterHelp(c_sCdUpHelpId, ":/resources/help/editor/cdup_button_help.html");
    m_spUi->AddResourceButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddResourceHelpId);
    wpHelpFactory->RegisterHelp(c_sAddResourceHelpId, ":/resources/help/editor/addresource_button_help.html");
    m_spUi->AddWebResourceButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddWebResourceHelpId);
    wpHelpFactory->RegisterHelp(c_sAddWebResourceHelpId, ":/resources/help/editor/addwebresource_button_help.html");
    m_spUi->RemoveResourceButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRemoveResourceHelpId);
    wpHelpFactory->RegisterHelp(c_sRemoveResourceHelpId, ":/resources/help/editor/removeresource_button_help.html");
    m_spUi->SourceButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sImageSourceHelpId);
    wpHelpFactory->RegisterHelp(c_sImageSourceHelpId, ":/resources/help/editor/source_button_help.html");
    m_spUi->TitleCardButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sTitleCardHelpId);
    wpHelpFactory->RegisterHelp(c_sTitleCardHelpId, ":/resources/help/editor/titlecard_button_help.html");
    m_spUi->TagsButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sTagsHelpId);
    wpHelpFactory->RegisterHelp(c_sTagsHelpId, ":/resources/help/editor/tags_button_help.html");
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
    m_spUi->AddMetronomeCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddMetronomeHelpId);
    wpHelpFactory->RegisterHelp(c_sAddMetronomeHelpId, ":/resources/help/editor/addmetronome_button_help.html");
    m_spUi->AddTimerCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddTimerHelpId);
    wpHelpFactory->RegisterHelp(c_sAddTimerHelpId, ":/resources/help/editor/addtimer_button_help.html");
    m_spUi->AddThreadCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddThreadHelpId);
    wpHelpFactory->RegisterHelp(c_sAddThreadHelpId, ":/resources/help/editor/addthread_button_help.html");
    m_spUi->AddNotificationCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddNotificationHelpId);
    wpHelpFactory->RegisterHelp(c_sAddNotificationHelpId, ":/resources/help/editor/addnotification_button_help.html");
    m_spUi->AddDeviceCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddDeviceHelpId);
    wpHelpFactory->RegisterHelp(c_sAddDeviceHelpId, ":/resources/help/editor/adddevice_button_help.html");

    m_spUi->AddImageCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosImageHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosImageHelpId, ":/resources/help/editor/add_eos_image_button_help.html");
    m_spUi->AddAutioCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosAudioHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosAudioHelpId, ":/resources/help/editor/add_eos_audio_button_help.html");
    m_spUi->AddSayCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosSayHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosSayHelpId, ":/resources/help/editor/add_eos_say_button_help.html");
    m_spUi->AddPromptCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosPromptHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosPromptHelpId, ":/resources/help/editor/add_eos_prompt_button_help.html");
    m_spUi->AddTimerCode2->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosTimerHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosTimerHelpId, ":/resources/help/editor/add_eos_timer_button_help.html");
    m_spUi->AddChoiceCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosChioceHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosChioceHelpId, ":/resources/help/editor/add_eos_choice_button_help.html");
    m_spUi->AddIfCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosIfHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosIfHelpId, ":/resources/help/editor/add_eos_if_button_help.html");
    m_spUi->AddNotificationCreateCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosNotCreateHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosNotCreateHelpId, ":/resources/help/editor/add_eos_notification_button_help.html");
    m_spUi->AddNotificationCloseCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosNotCloseHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosNotCloseHelpId, ":/resources/help/editor/add_eos_notification_button_help.html");
    m_spUi->AddEvalCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosEvalHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosEvalHelpId, ":/resources/help/editor/add_eos_eval_button_help.html");
    m_spUi->AddGotoCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosGotoHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosGotoHelpId, ":/resources/help/editor/add_eos_goto_button_help.html");
    m_spUi->AddEndCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosEndHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosEndHelpId, ":/resources/help/editor/add_eos_end_button_help.html");
    m_spUi->AddEnableCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosEnableHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosEnableHelpId, ":/resources/help/editor/add_eos_enable_button_help.html");
    m_spUi->AddDisableCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosDisableHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosDisableHelpId, ":/resources/help/editor/add_eos_disable_button_help.html");
    m_spUi->RemoveCode->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddEosRemoveHelpId);
    wpHelpFactory->RegisterHelp(c_sAddEosRemoveHelpId, ":/resources/help/editor/remove_eos_button_help.html");

    m_spUi->DebugLayoutButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sDebugSeqHelpId);
    m_spUi->StopDebugLayoutButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sDebugSeqHelpId);
    wpHelpFactory->RegisterHelp(c_sDebugSeqHelpId, ":/resources/help/editor/sequence/startdebug_button_help.html");
    m_spUi->NewSequence->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddSeqHelpId);
    wpHelpFactory->RegisterHelp(c_sAddSeqHelpId, ":/resources/help/editor/sequence/createsequence_button_help.html");
    m_spUi->SequenceProperties->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sEditSeqHelpId);
    wpHelpFactory->RegisterHelp(c_sEditSeqHelpId, ":/resources/help/editor/sequence/editsequence_button_help.html");
    m_spUi->AddSequenceElement->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddSeqElemHelpId);
    wpHelpFactory->RegisterHelp(c_sAddSeqElemHelpId, ":/resources/help/editor/sequence/add_sequenceelem_button_help.html");
    m_spUi->RemoveSelectedSequenceElements->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRemoveSeqElemsHelpId);
    wpHelpFactory->RegisterHelp(c_sRemoveSeqElemsHelpId, ":/resources/help/editor/sequence/remove_sequenceelem_button_help.html");
    m_spUi->AddSequenceLayer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddSeqLayerHelpId);
    wpHelpFactory->RegisterHelp(c_sAddSeqLayerHelpId, ":/resources/help/editor/sequence/add_sequencelayer_button_help.html");
    m_spUi->RemoveSelectedSequenceLayer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRemoveSeqLayerHelpId);
    wpHelpFactory->RegisterHelp(c_sRemoveSeqLayerHelpId, ":/resources/help/editor/sequence/remove_sequencelayer_button_help.html");

    m_spUi->AddDialogue->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddDialogueHelpId);
    wpHelpFactory->RegisterHelp(c_sAddDialogueHelpId, ":/resources/help/editor/add_dialog_button_help.html");
    m_spUi->AddDialogueFrament->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddCondDialogueHelpId);
    wpHelpFactory->RegisterHelp(c_sAddCondDialogueHelpId, ":/resources/help/editor/add_conditionaldialog_button_help.html");
    m_spUi->AddDialogueCategoryButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddDialogueCategoryHelpId);
    wpHelpFactory->RegisterHelp(c_sAddDialogueCategoryHelpId, ":/resources/help/editor/add_dialogcategory_button_help.html");
    m_spUi->RemoveDialogue->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRemoveDialogueHelpId);
    wpHelpFactory->RegisterHelp(c_sRemoveDialogueHelpId, ":/resources/help/editor/remove_dialog_button_help.html");
    m_spUi->EditDialogueContent->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sEditDialogueHelpId);
    wpHelpFactory->RegisterHelp(c_sEditDialogueHelpId, ":/resources/help/editor/edit_dialog_button_help.html");
    m_spUi->EditDialogueTags->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sEditDialogueTagsHelpId);
    wpHelpFactory->RegisterHelp(c_sEditDialogueTagsHelpId, ":/resources/help/editor/edit_dialogtags_button_help.html");
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
  m_spUi->pSequenceEditorContainer->hide();
  m_spUi->pDialogueEditorContainer->hide();

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
  m_spUi->pNodeDebugStack->setCurrentIndex(0);

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
  // No buttons needed
  //m_spUi->pProjectSettingsEditorContainer->show();

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
void CEditorActionBar::ShowSequenceEditorActionBar()
{
  HideAllBars();
  m_spUi->pSequenceEditorContainer->show();
  m_spUi->DebugLayoutButton->show();
  m_spUi->StopDebugLayoutButton->hide();

  m_iCurrentDisplayType = EEditorWidget::ePatternEditor;
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::ShowDialogueActionBar()
{
  HideAllBars();
  m_spUi->pDialogueEditorContainer->show();
  m_iCurrentDisplayType = EEditorWidget::eDialogueEditor;
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
    m_spUi->UndoButton->SetShortcut(m_spSettings->keyBinding("Undo"));
    m_spUi->RedoButton->SetShortcut(m_spSettings->keyBinding("Redo"));
    m_spUi->ExportButton->SetShortcut(m_spSettings->keyBinding("Export"));
    m_spUi->ToolsButton->SetShortcut(m_spSettings->keyBinding("Tools"));
  }
  else
  {
    m_spUi->TreeViewButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->CdUpButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));
    m_spUi->AddResourceButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(3)));
    m_spUi->AddWebResourceButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(4)));
    m_spUi->RemoveResourceButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(5)));
    m_spUi->SourceButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(6)));
    m_spUi->TitleCardButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(7)));
    m_spUi->TagsButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(8)));
    m_spUi->MapButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(9)));

    m_spUi->PlayButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->PauseButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));
    m_spUi->StopButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(3)));

    m_spUi->DebugNodeButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->StopDebugNodeButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));
    m_spUi->NextSceneButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(3)));
    m_spUi->AddNodeButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(4)));
    m_spUi->RemoveNodeButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(5)));

    m_spUi->DebugButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->StopDebugButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));
    m_spUi->AddShowImageCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(3)));
    m_spUi->AddShowBackgroundCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(4)));
    m_spUi->AddTextCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(5)));
    m_spUi->AddShowIconCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(6)));
    m_spUi->AddTimerCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(7)));
    m_spUi->AddMetronomeCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(8)));
    m_spUi->AddThreadCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(9)));
    //m_spUi->AddNotificationCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(9)));
    //m_spUi->AddDeviceCode->SetShortcut(m_spSettings->keyBinding(sKey.arg(9)));

    m_spUi->DebugLayoutButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->StopDebugLayoutButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));
    m_spUi->NewSequence->SetShortcut(m_spSettings->keyBinding(sKey.arg(3)));
    m_spUi->SequenceProperties->SetShortcut(m_spSettings->keyBinding(sKey.arg(4)));
    m_spUi->AddSequenceLayer->SetShortcut(m_spSettings->keyBinding(sKey.arg(5)));
    m_spUi->RemoveSelectedSequenceLayer->SetShortcut(m_spSettings->keyBinding(sKey.arg(6)));
    m_spUi->AddSequenceElement->SetShortcut(m_spSettings->keyBinding(sKey.arg(7)));
    m_spUi->RemoveSelectedSequenceElements->SetShortcut(m_spSettings->keyBinding(sKey.arg(8)));

    m_spUi->AddDialogue->SetShortcut(m_spSettings->keyBinding(sKey.arg(1)));
    m_spUi->AddDialogueFrament->SetShortcut(m_spSettings->keyBinding(sKey.arg(2)));
    m_spUi->AddDialogueCategoryButton->SetShortcut(m_spSettings->keyBinding(sKey.arg(3)));
    m_spUi->RemoveDialogue->SetShortcut(m_spSettings->keyBinding(sKey.arg(4)));
    m_spUi->EditDialogueContent->SetShortcut(m_spSettings->keyBinding(sKey.arg(5)));
    m_spUi->EditDialogueTags->SetShortcut(m_spSettings->keyBinding(sKey.arg(6)));
  }
}
