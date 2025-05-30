cmake_minimum_required(VERSION 3.5)

#---------------------------------------------------------------------------------------
# find Packages
#---------------------------------------------------------------------------------------
macro(FindRequiredQtPackages)
  find_package(QT NAMES Qt6 Qt5 COMPONENTS
    Core
    Multimedia
    MultimediaWidgets
    Network
    Qml
    Quick
    QuickControls2
    QuickWidgets
    Svg
    Widgets
    WebChannel
    Xml REQUIRED)
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Core
    Multimedia
    MultimediaWidgets
    Network
    Qml
    Quick
    QuickControls2
    QuickWidgets
    Svg
    Widgets
    WebChannel
    Xml REQUIRED)

  if (KDE_DEBUG)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Test REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
  endif()

  if (WIN32)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS WinExtras REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS WinExtras REQUIRED)
  elseif (ANDROID)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS AndroidExtras REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS AndroidExtras REQUIRED)
  endif()
endmacro(FindRequiredQtPackages)

#---------------------------------------------------------------------------------------
# Create Project
#---------------------------------------------------------------------------------------
macro(CreateJOIPProject JOIP_PROJECT_NAME)
  #---------------------------------------------------------------------------------------
  # Sources
  #---------------------------------------------------------------------------------------
  set(JOIPSources ${CMAKE_SOURCE_DIR}/src)
  set(Sources
    ${JOIPSources}/Application.cpp
    ${JOIPSources}/Application.h
    ${JOIPSources}/Constants.h
    ${JOIPSources}/CreditsScreen.cpp
    ${JOIPSources}/CreditsScreen.h
    ${JOIPSources}/CreditsScreen.ui
    ${JOIPSources}/ClipboardQmlWrapper.cpp
    ${JOIPSources}/ClipboardQmlWrapper.h
    ${JOIPSources}/DownloadScreen.cpp
    ${JOIPSources}/DownloadScreen.h
    ${JOIPSources}/DownloadScreen.ui
    ${JOIPSources}/Editor/CommandChangeTag.cpp
    ${JOIPSources}/Editor/CommandChangeTag.h
    ${JOIPSources}/Editor/DialogueEditor/CommandAddNewDialogueFile.cpp
    ${JOIPSources}/Editor/DialogueEditor/CommandAddNewDialogueFile.h
    ${JOIPSources}/Editor/DialogueEditor/CommandAddRemoveDialogueTags.cpp
    ${JOIPSources}/Editor/DialogueEditor/CommandAddRemoveDialogueTags.h
    ${JOIPSources}/Editor/DialogueEditor/CommandAddRemoveNode.cpp
    ${JOIPSources}/Editor/DialogueEditor/CommandAddRemoveNode.h
    ${JOIPSources}/Editor/DialogueEditor/CommandChangeModelViaGui.cpp
    ${JOIPSources}/Editor/DialogueEditor/CommandChangeModelViaGui.h
    ${JOIPSources}/Editor/DialogueEditor/CommandChangeParameters.cpp
    ${JOIPSources}/Editor/DialogueEditor/CommandChangeParameters.h
    ${JOIPSources}/Editor/DialogueEditor/DialogueEditorDelegate.cpp
    ${JOIPSources}/Editor/DialogueEditor/DialogueEditorDelegate.h
    ${JOIPSources}/Editor/DialogueEditor/DialogueEditorSortFilterProxyModel.cpp
    ${JOIPSources}/Editor/DialogueEditor/DialogueEditorSortFilterProxyModel.h
    ${JOIPSources}/Editor/DialogueEditor/DialogueEditorTreeItem.cpp
    ${JOIPSources}/Editor/DialogueEditor/DialogueEditorTreeItem.h
    ${JOIPSources}/Editor/DialogueEditor/DialogueEditorTreeModel.cpp
    ${JOIPSources}/Editor/DialogueEditor/DialogueEditorTreeModel.h
    ${JOIPSources}/Editor/DialogueEditor/DialoguePropertyEditor.cpp
    ${JOIPSources}/Editor/DialogueEditor/DialoguePropertyEditor.h
    ${JOIPSources}/Editor/DialogueEditor/DialoguePropertyEditor.ui
    ${JOIPSources}/Editor/DialogueEditor/DialogueTagsEditorOverlay.cpp
    ${JOIPSources}/Editor/DialogueEditor/DialogueTagsEditorOverlay.h
    ${JOIPSources}/Editor/DialogueEditor/DialogueTagsEditorOverlay.ui
    ${JOIPSources}/Editor/EditorActionBar.cpp
    ${JOIPSources}/Editor/EditorActionBar.h
    ${JOIPSources}/Editor/EditorActionBar.ui
    ${JOIPSources}/Editor/EditorChoiceScreen.cpp
    ${JOIPSources}/Editor/EditorChoiceScreen.h
    ${JOIPSources}/Editor/EditorChoiceScreen.ui
    ${JOIPSources}/Editor/EditorCommandIds.h
    ${JOIPSources}/Editor/EditorEditableFileModel.cpp
    ${JOIPSources}/Editor/EditorEditableFileModel.h
    ${JOIPSources}/Editor/EditorJobWorker.cpp
    ${JOIPSources}/Editor/EditorJobWorker.h
    ${JOIPSources}/Editor/EditorJobs/EditorExportJob.cpp
    ${JOIPSources}/Editor/EditorJobs/EditorExportJob.h
    ${JOIPSources}/Editor/EditorJobs/EditorImageCompressionJob.cpp
    ${JOIPSources}/Editor/EditorJobs/EditorImageCompressionJob.h
    ${JOIPSources}/Editor/EditorJobs/EditorJobTypes.h
    ${JOIPSources}/Editor/EditorJobs/IEditorJobStateListener.h
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutBase.cpp
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutBase.h
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutClassic.cpp
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutClassic.h
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutClassic.ui
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutCompact.cpp
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutCompact.h
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutCompact.ui
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutModern.cpp
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutModern.h
    ${JOIPSources}/Editor/EditorLayouts/EditorLayoutModern.ui
    ${JOIPSources}/Editor/EditorLayouts/IEditorLayoutViewProvider.h
    ${JOIPSources}/Editor/EditorMainScreen.cpp
    ${JOIPSources}/Editor/EditorMainScreen.h
    ${JOIPSources}/Editor/EditorMainScreen.ui
    ${JOIPSources}/Editor/EditorModel.cpp
    ${JOIPSources}/Editor/EditorModel.h
    ${JOIPSources}/Editor/IEditorTool.h
    ${JOIPSources}/Editor/EditorWidgets/EditorCodeWidget.cpp
    ${JOIPSources}/Editor/EditorWidgets/EditorCodeWidget.h
    ${JOIPSources}/Editor/EditorWidgets/EditorCodeWidget.ui
    ${JOIPSources}/Editor/EditorWidgets/EditorDebuggableWidget.cpp
    ${JOIPSources}/Editor/EditorWidgets/EditorDebuggableWidget.h
    ${JOIPSources}/Editor/EditorWidgets/EditorDialogueWidget.cpp
    ${JOIPSources}/Editor/EditorWidgets/EditorDialogueWidget.h
    ${JOIPSources}/Editor/EditorWidgets/EditorDialogueWidget.ui
    ${JOIPSources}/Editor/EditorWidgets/EditorProjectSettingsWidget.cpp
    ${JOIPSources}/Editor/EditorWidgets/EditorProjectSettingsWidget.h
    ${JOIPSources}/Editor/EditorWidgets/EditorProjectSettingsWidget.ui
    ${JOIPSources}/Editor/EditorWidgets/EditorResourceDisplayWidget.cpp
    ${JOIPSources}/Editor/EditorWidgets/EditorResourceDisplayWidget.h
    ${JOIPSources}/Editor/EditorWidgets/EditorResourceDisplayWidget.ui
    ${JOIPSources}/Editor/EditorWidgets/EditorResourceWidget.cpp
    ${JOIPSources}/Editor/EditorWidgets/EditorResourceWidget.h
    ${JOIPSources}/Editor/EditorWidgets/EditorResourceWidget.ui
    ${JOIPSources}/Editor/EditorWidgets/EditorSceneNodeWidget.cpp
    ${JOIPSources}/Editor/EditorWidgets/EditorSceneNodeWidget.h
    ${JOIPSources}/Editor/EditorWidgets/EditorSceneNodeWidget.ui
    ${JOIPSources}/Editor/EditorWidgets/EditorSequenceEditorWidget.cpp
    ${JOIPSources}/Editor/EditorWidgets/EditorSequenceEditorWidget.h
    ${JOIPSources}/Editor/EditorWidgets/EditorSequenceEditorWidget.ui
    ${JOIPSources}/Editor/EditorWidgets/EditorWidgetBase.cpp
    ${JOIPSources}/Editor/EditorWidgets/EditorWidgetBase.h
    ${JOIPSources}/Editor/EditorWidgetRegistry.h
    ${JOIPSources}/Editor/EditorWidgetTypes.h
    ${JOIPSources}/Editor/NodeEditor/CommandChangeSelection.cpp
    ${JOIPSources}/Editor/NodeEditor/CommandChangeSelection.h
    ${JOIPSources}/Editor/NodeEditor/CommandConnectionAdded.cpp
    ${JOIPSources}/Editor/NodeEditor/CommandConnectionAdded.h
    ${JOIPSources}/Editor/NodeEditor/CommandConnectionRemoved.cpp
    ${JOIPSources}/Editor/NodeEditor/CommandConnectionRemoved.h
    ${JOIPSources}/Editor/NodeEditor/CommandNodeAdded.cpp
    ${JOIPSources}/Editor/NodeEditor/CommandNodeAdded.h
    ${JOIPSources}/Editor/NodeEditor/CommandNodeEdited.cpp
    ${JOIPSources}/Editor/NodeEditor/CommandNodeEdited.h
    ${JOIPSources}/Editor/NodeEditor/CommandNodeMoved.cpp
    ${JOIPSources}/Editor/NodeEditor/CommandNodeMoved.h
    ${JOIPSources}/Editor/NodeEditor/CommandNodeRemoved.cpp
    ${JOIPSources}/Editor/NodeEditor/CommandNodeRemoved.h
    ${JOIPSources}/Editor/NodeEditor/EditorNodeModelBase.cpp
    ${JOIPSources}/Editor/NodeEditor/EditorNodeModelBase.h
    ${JOIPSources}/Editor/NodeEditor/EndNodeModel.cpp
    ${JOIPSources}/Editor/NodeEditor/EndNodeModel.h
    ${JOIPSources}/Editor/NodeEditor/FlowScene.cpp
    ${JOIPSources}/Editor/NodeEditor/FlowScene.h
    ${JOIPSources}/Editor/NodeEditor/FlowView.cpp
    ${JOIPSources}/Editor/NodeEditor/FlowView.h
    ${JOIPSources}/Editor/NodeEditor/IUndoStackAwareModel.h
    ${JOIPSources}/Editor/NodeEditor/NodeEditorRegistry.cpp
    ${JOIPSources}/Editor/NodeEditor/NodeEditorRegistry.h
    ${JOIPSources}/Editor/NodeEditor/PathMergerModel.cpp
    ${JOIPSources}/Editor/NodeEditor/PathMergerModel.h
    ${JOIPSources}/Editor/NodeEditor/PathSplitterModel.cpp
    ${JOIPSources}/Editor/NodeEditor/PathSplitterModel.h
    ${JOIPSources}/Editor/NodeEditor/PathSplitterModelWidget.cpp
    ${JOIPSources}/Editor/NodeEditor/PathSplitterModelWidget.h
    ${JOIPSources}/Editor/NodeEditor/PathSplitterModelWidget.ui
    ${JOIPSources}/Editor/NodeEditor/SceneNodeModel.cpp
    ${JOIPSources}/Editor/NodeEditor/SceneNodeModel.h
    ${JOIPSources}/Editor/NodeEditor/SceneNodeModelWidget.cpp
    ${JOIPSources}/Editor/NodeEditor/SceneNodeModelWidget.h
    ${JOIPSources}/Editor/NodeEditor/SceneNodeModelWidget.ui
    ${JOIPSources}/Editor/NodeEditor/SceneTranstitionData.cpp
    ${JOIPSources}/Editor/NodeEditor/SceneTranstitionData.h
    ${JOIPSources}/Editor/NodeEditor/StartNodeModel.cpp
    ${JOIPSources}/Editor/NodeEditor/StartNodeModel.h
    ${JOIPSources}/Editor/Project/AchievementWidget.cpp
    ${JOIPSources}/Editor/Project/AchievementWidget.h
    ${JOIPSources}/Editor/Project/CommandChangeAchievements.cpp
    ${JOIPSources}/Editor/Project/CommandChangeAchievements.h
    ${JOIPSources}/Editor/Project/CommandChangeDescribtion.cpp
    ${JOIPSources}/Editor/Project/CommandChangeDescribtion.h
    ${JOIPSources}/Editor/Project/CommandChangeEmitterCount.cpp
    ${JOIPSources}/Editor/Project/CommandChangeEmitterCount.h
    ${JOIPSources}/Editor/Project/CommandChangeFetishes.cpp
    ${JOIPSources}/Editor/Project/CommandChangeFetishes.h
    ${JOIPSources}/Editor/Project/CommandChangeFont.cpp
    ${JOIPSources}/Editor/Project/CommandChangeFont.h
    ${JOIPSources}/Editor/Project/CommandChangeLayout.cpp
    ${JOIPSources}/Editor/Project/CommandChangeLayout.h
    ${JOIPSources}/Editor/Project/CommandChangeProjectName.cpp
    ${JOIPSources}/Editor/Project/CommandChangeProjectName.h
    ${JOIPSources}/Editor/Project/CommandChangeToyCmd.cpp
    ${JOIPSources}/Editor/Project/CommandChangeToyCmd.h
    ${JOIPSources}/Editor/Project/CommandChangeVersion.cpp
    ${JOIPSources}/Editor/Project/CommandChangeVersion.h
    ${JOIPSources}/Editor/Project/KinkCompleter.cpp
    ${JOIPSources}/Editor/Project/KinkCompleter.h
    ${JOIPSources}/Editor/Project/KinkSelectionOverlay.cpp
    ${JOIPSources}/Editor/Project/KinkSelectionOverlay.h
    ${JOIPSources}/Editor/Project/KinkSelectionOverlay.ui
    ${JOIPSources}/Editor/Project/KinkTreeItem.cpp
    ${JOIPSources}/Editor/Project/KinkTreeItem.h
    ${JOIPSources}/Editor/Project/KinkTreeModel.cpp
    ${JOIPSources}/Editor/Project/KinkTreeModel.h
    ${JOIPSources}/Editor/Project/KinkTreeSortFilterProxyModel.cpp
    ${JOIPSources}/Editor/Project/KinkTreeSortFilterProxyModel.h
    ${JOIPSources}/Editor/Project/SelectableResourceLabel.cpp
    ${JOIPSources}/Editor/Project/SelectableResourceLabel.h
    ${JOIPSources}/Editor/Resources/CommandAddResource.cpp
    ${JOIPSources}/Editor/Resources/CommandAddResource.h
    ${JOIPSources}/Editor/Resources/CommandChangeCurrentResource.cpp
    ${JOIPSources}/Editor/Resources/CommandChangeCurrentResource.h
    ${JOIPSources}/Editor/Resources/CommandChangeFilter.cpp
    ${JOIPSources}/Editor/Resources/CommandChangeFilter.h
    ${JOIPSources}/Editor/Resources/CommandChangeResourceData.cpp
    ${JOIPSources}/Editor/Resources/CommandChangeResourceData.h
    ${JOIPSources}/Editor/Resources/CommandChangeSource.cpp
    ${JOIPSources}/Editor/Resources/CommandChangeSource.h
    ${JOIPSources}/Editor/Resources/CommandChangeTags.cpp
    ${JOIPSources}/Editor/Resources/CommandChangeTags.h
    ${JOIPSources}/Editor/Resources/CommandChangeTitleCard.cpp
    ${JOIPSources}/Editor/Resources/CommandChangeTitleCard.h
    ${JOIPSources}/Editor/Resources/CommandRemoveResource.cpp
    ${JOIPSources}/Editor/Resources/CommandRemoveResource.h
    ${JOIPSources}/Editor/Resources/CompressJobSettingsOverlay.cpp
    ${JOIPSources}/Editor/Resources/CompressJobSettingsOverlay.h
    ${JOIPSources}/Editor/Resources/CompressJobSettingsOverlay.ui
    ${JOIPSources}/Editor/Resources/ResourceDetailView.cpp
    ${JOIPSources}/Editor/Resources/ResourceDetailView.h
    ${JOIPSources}/Editor/Resources/ResourceDetailViewFetcherThread.cpp
    ${JOIPSources}/Editor/Resources/ResourceDetailViewFetcherThread.h
    ${JOIPSources}/Editor/Resources/ResourceModelView.cpp
    ${JOIPSources}/Editor/Resources/ResourceModelView.h
    ${JOIPSources}/Editor/Resources/ResourceModelView.ui
    ${JOIPSources}/Editor/Resources/ResourceToolTip.cpp
    ${JOIPSources}/Editor/Resources/ResourceToolTip.h
    ${JOIPSources}/Editor/Resources/ResourceToolTip.ui
    ${JOIPSources}/Editor/Resources/ResourceTreeDelegate.cpp
    ${JOIPSources}/Editor/Resources/ResourceTreeDelegate.h
    ${JOIPSources}/Editor/Resources/ResourceTreeItem.cpp
    ${JOIPSources}/Editor/Resources/ResourceTreeItem.h
    ${JOIPSources}/Editor/Resources/ResourceTreeItemModel.cpp
    ${JOIPSources}/Editor/Resources/ResourceTreeItemModel.h
    ${JOIPSources}/Editor/Resources/ResourceTreeItemSortFilterProxyModel.cpp
    ${JOIPSources}/Editor/Resources/ResourceTreeItemSortFilterProxyModel.h
    ${JOIPSources}/Editor/Resources/TagsEditorOverlay.cpp
    ${JOIPSources}/Editor/Resources/TagsEditorOverlay.h
    ${JOIPSources}/Editor/Resources/TagsEditorOverlay.ui
    ${JOIPSources}/Editor/Resources/WebResourceOverlay.cpp
    ${JOIPSources}/Editor/Resources/WebResourceOverlay.h
    ${JOIPSources}/Editor/Resources/WebResourceOverlay.ui
    ${JOIPSources}/Editor/SequenceEditor/CommandAddNewSequence.cpp
    ${JOIPSources}/Editor/SequenceEditor/CommandAddNewSequence.h
    ${JOIPSources}/Editor/SequenceEditor/CommandChangeOpenedSequence.cpp
    ${JOIPSources}/Editor/SequenceEditor/CommandChangeOpenedSequence.h
    ${JOIPSources}/Editor/SequenceEditor/CommandChangeSequenceProperties.cpp
    ${JOIPSources}/Editor/SequenceEditor/CommandChangeSequenceProperties.h
    ${JOIPSources}/Editor/SequenceEditor/CommandModifyLayerProperties.cpp
    ${JOIPSources}/Editor/SequenceEditor/CommandModifyLayerProperties.h
    ${JOIPSources}/Editor/SequenceEditor/CommandModifyLayers.cpp
    ${JOIPSources}/Editor/SequenceEditor/CommandModifyLayers.h
    ${JOIPSources}/Editor/SequenceEditor/SequencePropertiesOverlay.cpp
    ${JOIPSources}/Editor/SequenceEditor/SequencePropertiesOverlay.h
    ${JOIPSources}/Editor/SequenceEditor/SequencePropertiesOverlay.ui
    ${JOIPSources}/Editor/SequenceEditor/SequenceEmentList.cpp
    ${JOIPSources}/Editor/SequenceEditor/SequenceEmentList.h
    ${JOIPSources}/Editor/SequenceEditor/SqeuenceElementListModel.cpp
    ${JOIPSources}/Editor/SequenceEditor/SqeuenceElementListModel.h
    ${JOIPSources}/Editor/SequenceEditor/SequencePreviewWidget.cpp
    ${JOIPSources}/Editor/SequenceEditor/SequencePreviewWidget.h
    ${JOIPSources}/Editor/SequenceEditor/SequencePreviewWidget.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionConfigOverlay.cpp
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionConfigOverlay.h
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetEval.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetLinearToy.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetPauseAudio.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetPlayAudio.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetPlayVideo.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetRotateToy.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetRunScript.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgets.cpp
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgets.h
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetShowMedia.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetShowText.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetSingleBeat.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetStartPattern.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetStopAudio.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineSeqeunceInstructionWidgetVibrate.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidget.cpp
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidget.h
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidget.ui
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetBackground.cpp
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetBackground.h
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetControls.cpp
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetControls.h
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetLayer.cpp
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetLayer.h
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetLayerBackground.cpp
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetLayerBackground.h
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetOverlay.cpp
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetOverlay.h
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetUtils.cpp
    ${JOIPSources}/Editor/SequenceEditor/TimelineWidgetUtils.h
    ${JOIPSources}/Editor/Script/BackgroundSnippetOverlay.cpp
    ${JOIPSources}/Editor/Script/BackgroundSnippetOverlay.h
    ${JOIPSources}/Editor/Script/BackgroundSnippetOverlay.ui
    ${JOIPSources}/Editor/Script/CodeDisplayDefaultEditorImpl.cpp
    ${JOIPSources}/Editor/Script/CodeDisplayDefaultEditorImpl.h
    ${JOIPSources}/Editor/Script/CodeDisplayEosEditorImpl.cpp
    ${JOIPSources}/Editor/Script/CodeDisplayEosEditorImpl.h
    ${JOIPSources}/Editor/Script/CodeDisplayLayoutEditorImpl.cpp
    ${JOIPSources}/Editor/Script/CodeDisplayLayoutEditorImpl.h
    ${JOIPSources}/Editor/Script/CodeDisplayWidget.cpp
    ${JOIPSources}/Editor/Script/CodeDisplayWidget.h
    ${JOIPSources}/Editor/Script/CodeDisplayWidget.ui
    ${JOIPSources}/Editor/Script/CodeSnippetOverlayBase.cpp
    ${JOIPSources}/Editor/Script/CodeSnippetOverlayBase.h
    ${JOIPSources}/Editor/Script/CommandChangeOpenedScript.cpp
    ${JOIPSources}/Editor/Script/CommandChangeOpenedScript.h
    ${JOIPSources}/Editor/Script/CommandInsertEosCommand.cpp
    ${JOIPSources}/Editor/Script/CommandInsertEosCommand.h
    ${JOIPSources}/Editor/Script/CommandRemoveEosCommand.cpp
    ${JOIPSources}/Editor/Script/CommandRemoveEosCommand.h
    ${JOIPSources}/Editor/Script/CommandScriptContentChange.cpp
    ${JOIPSources}/Editor/Script/CommandScriptContentChange.h
    ${JOIPSources}/Editor/Script/CommandToggledEosCommand.cpp
    ${JOIPSources}/Editor/Script/CommandToggledEosCommand.h
    ${JOIPSources}/Editor/Script/CommandUpdateEosCommand.cpp
    ${JOIPSources}/Editor/Script/CommandUpdateEosCommand.h
    ${JOIPSources}/Editor/Script/DeviceSnippetOverlay.cpp
    ${JOIPSources}/Editor/Script/DeviceSnippetOverlay.h
    ${JOIPSources}/Editor/Script/DeviceSnippetOverlay.ui
    ${JOIPSources}/Editor/Script/EosCommandModels.cpp
    ${JOIPSources}/Editor/Script/EosCommandModels.h
    ${JOIPSources}/Editor/Script/EosCommandWidgets.cpp
    ${JOIPSources}/Editor/Script/EosCommandWidgets.h
    ${JOIPSources}/Editor/Script/EosCommandWidgetAudio.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetChoice.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetEval.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetGoto.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetIf.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetImage.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetNotificationClose.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetNotificationCreate.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetPrompt.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetSay.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetScene.ui
    ${JOIPSources}/Editor/Script/EosCommandWidgetTimer.ui
    ${JOIPSources}/Editor/Script/EosScriptEditorView.cpp
    ${JOIPSources}/Editor/Script/EosScriptEditorView.h
    ${JOIPSources}/Editor/Script/EosScriptModel.cpp
    ${JOIPSources}/Editor/Script/EosScriptModel.h
    ${JOIPSources}/Editor/Script/EosScriptModelItem.cpp
    ${JOIPSources}/Editor/Script/EosScriptModelItem.h
    ${JOIPSources}/Editor/Script/EosScriptOverlayDelegate.cpp
    ${JOIPSources}/Editor/Script/EosScriptOverlayDelegate.h
    ${JOIPSources}/Editor/Script/ICodeDisplayWidgetImpl.h
    ${JOIPSources}/Editor/Script/IconSnippetOverlay.cpp
    ${JOIPSources}/Editor/Script/IconSnippetOverlay.h
    ${JOIPSources}/Editor/Script/IconSnippetOverlay.ui
    ${JOIPSources}/Editor/Script/IScriptEditorAddons.h
    ${JOIPSources}/Editor/Script/MetronomeSnippetOverlay.cpp
    ${JOIPSources}/Editor/Script/MetronomeSnippetOverlay.h
    ${JOIPSources}/Editor/Script/MetronomeSnippetOverlay.ui
    ${JOIPSources}/Editor/Script/NotificationSnippetOverlay.cpp
    ${JOIPSources}/Editor/Script/NotificationSnippetOverlay.h
    ${JOIPSources}/Editor/Script/NotificationSnippetOverlay.ui
    ${JOIPSources}/Editor/Script/ResourceSnippetOverlay.cpp
    ${JOIPSources}/Editor/Script/ResourceSnippetOverlay.h
    ${JOIPSources}/Editor/Script/ResourceSnippetOverlay.ui
    ${JOIPSources}/Editor/Script/ScriptCompleterFileProcessors.cpp
    ${JOIPSources}/Editor/Script/ScriptCompleterFileProcessors.h
    ${JOIPSources}/Editor/Script/ScriptEditorAddonWidgets.cpp
    ${JOIPSources}/Editor/Script/ScriptEditorAddonWidgets.h
    ${JOIPSources}/Editor/Script/ScriptEditorCodeToolTip.cpp
    ${JOIPSources}/Editor/Script/ScriptEditorCodeToolTip.h
    ${JOIPSources}/Editor/Script/ScriptEditorCompleter.cpp
    ${JOIPSources}/Editor/Script/ScriptEditorCompleter.h
    ${JOIPSources}/Editor/Script/ScriptEditorCompleterModel.cpp
    ${JOIPSources}/Editor/Script/ScriptEditorCompleterModel.h
    ${JOIPSources}/Editor/Script/ScriptEditorKeyHandler.cpp
    ${JOIPSources}/Editor/Script/ScriptEditorKeyHandler.h
    ${JOIPSources}/Editor/Script/ScriptEditorWidget.cpp
    ${JOIPSources}/Editor/Script/ScriptEditorWidget.h
    ${JOIPSources}/Editor/Script/ScriptFooterArea.ui
    ${JOIPSources}/Editor/Script/TextSnippetOverlay.cpp
    ${JOIPSources}/Editor/Script/TextSnippetOverlay.h
    ${JOIPSources}/Editor/Script/TextSnippetOverlay.ui
    ${JOIPSources}/Editor/Script/ThreadSnippetOverlay.cpp
    ${JOIPSources}/Editor/Script/ThreadSnippetOverlay.h
    ${JOIPSources}/Editor/Script/ThreadSnippetOverlay.ui
    ${JOIPSources}/Editor/Script/TimerSnippetOverlay.cpp
    ${JOIPSources}/Editor/Script/TimerSnippetOverlay.h
    ${JOIPSources}/Editor/Script/TimerSnippetOverlay.ui
    ${JOIPSources}/Editor/Tutorial/ClassicTutorialStateSwitchHandler.cpp
    ${JOIPSources}/Editor/Tutorial/ClassicTutorialStateSwitchHandler.h
    ${JOIPSources}/Editor/Tutorial/CodeWidgetTutorialStateSwitchHandler.cpp
    ${JOIPSources}/Editor/Tutorial/CodeWidgetTutorialStateSwitchHandler.h
    ${JOIPSources}/Editor/Tutorial/CommandBackground.cpp
    ${JOIPSources}/Editor/Tutorial/CommandBackground.h
    ${JOIPSources}/Editor/Tutorial/CommandClickFilter.cpp
    ${JOIPSources}/Editor/Tutorial/CommandClickFilter.h
    ${JOIPSources}/Editor/Tutorial/CommandEnd.cpp
    ${JOIPSources}/Editor/Tutorial/CommandEnd.h
    ${JOIPSources}/Editor/Tutorial/CommandHighlight.cpp
    ${JOIPSources}/Editor/Tutorial/CommandHighlight.h
    ${JOIPSources}/Editor/Tutorial/CommandText.cpp
    ${JOIPSources}/Editor/Tutorial/CommandText.h
    ${JOIPSources}/Editor/Tutorial/CompactTutorialStateSwitchHandler.cpp
    ${JOIPSources}/Editor/Tutorial/CompactTutorialStateSwitchHandler.h
    ${JOIPSources}/Editor/Tutorial/EditorTutorialOverlay.cpp
    ${JOIPSources}/Editor/Tutorial/EditorTutorialOverlay.h
    ${JOIPSources}/Editor/Tutorial/ITutorialStateSwitchHandler.cpp
    ${JOIPSources}/Editor/Tutorial/ITutorialStateSwitchHandler.h
    ${JOIPSources}/Editor/Tutorial/ModernTutorialStateSwitchHandler.cpp
    ${JOIPSources}/Editor/Tutorial/ModernTutorialStateSwitchHandler.h
    ${JOIPSources}/Editor/Tutorial/ProjectSettingsTutorialStateSwitchHandler.cpp
    ${JOIPSources}/Editor/Tutorial/ProjectSettingsTutorialStateSwitchHandler.h
    ${JOIPSources}/Editor/Tutorial/ResourceDisplayTutorialStateSwitchHandler.cpp
    ${JOIPSources}/Editor/Tutorial/ResourceDisplayTutorialStateSwitchHandler.h
    ${JOIPSources}/Editor/Tutorial/ResourceTutorialStateSwitchHandler.cpp
    ${JOIPSources}/Editor/Tutorial/ResourceTutorialStateSwitchHandler.h
    ${JOIPSources}/Editor/Tutorial/SceneNodeWidgetTutorialStateSwitchHandler.cpp
    ${JOIPSources}/Editor/Tutorial/SceneNodeWidgetTutorialStateSwitchHandler.h
    ${JOIPSources}/Editor/Tutorial/TutorialStateSwitchHandlerMainBase.cpp
    ${JOIPSources}/Editor/Tutorial/TutorialStateSwitchHandlerMainBase.h
    ${JOIPSources}/EditorScreen.cpp
    ${JOIPSources}/EditorScreen.h
    ${JOIPSources}/EditorScreen.ui
    ${JOIPSources}/Enums.h
    ${JOIPSources}/IAppStateScreen.h
    ${JOIPSources}/MainScreen.cpp
    ${JOIPSources}/MainScreen.h
    ${JOIPSources}/MainScreen.ui
    ${JOIPSources}/MainWindow.cpp
    ${JOIPSources}/MainWindow.h
    ${JOIPSources}/MainWindow.ui
    ${JOIPSources}/MainWindowFactory.cpp
    ${JOIPSources}/MainWindowFactory.h
    ${JOIPSources}/Player/InformationWidget.cpp
    ${JOIPSources}/Player/InformationWidget.h
    ${JOIPSources}/Player/InformationWidget.ui
    ${JOIPSources}/Player/MetronomePaintedWidget.cpp
    ${JOIPSources}/Player/MetronomePaintedWidget.h
    ${JOIPSources}/Player/ProjectDialogueManager.cpp
    ${JOIPSources}/Player/ProjectDialogueManager.h
    ${JOIPSources}/Player/ProjectEventTarget.cpp
    ${JOIPSources}/Player/ProjectEventTarget.h
    ${JOIPSources}/Player/ProjectNotificationManager.cpp
    ${JOIPSources}/Player/ProjectNotificationManager.h
    ${JOIPSources}/Player/ProjectSavegameManager.cpp
    ${JOIPSources}/Player/ProjectSavegameManager.h
    ${JOIPSources}/Player/ProjectSceneManager.cpp
    ${JOIPSources}/Player/ProjectSceneManager.h
    ${JOIPSources}/Player/ProjectSoundManager.cpp
    ${JOIPSources}/Player/ProjectSoundManager.h
    ${JOIPSources}/Player/SceneNodeResolver.cpp
    ${JOIPSources}/Player/SceneNodeResolver.h
    ${JOIPSources}/Player/SceneMainScreen.cpp
    ${JOIPSources}/Player/SceneMainScreen.h
    ${JOIPSources}/Player/SceneMainScreen.ui
    ${JOIPSources}/Player/TeaseDeviceController.cpp
    ${JOIPSources}/Player/TeaseDeviceController.h
    ${JOIPSources}/Player/TeaseStorage.cpp
    ${JOIPSources}/Player/TeaseStorage.h
    ${JOIPSources}/Player/TextBoxWidget.cpp
    ${JOIPSources}/Player/TextBoxWidget.h
    ${JOIPSources}/Player/TextBoxWidget.ui
    ${JOIPSources}/Player/TimerDisplayWidget.cpp
    ${JOIPSources}/Player/TimerDisplayWidget.h
    ${JOIPSources}/Player/TimerDisplayWidget.ui
    ${JOIPSources}/Player/TimerWidget.cpp
    ${JOIPSources}/Player/TimerWidget.h
    ${JOIPSources}/SVersion.h
    ${JOIPSources}/SceneScreen.cpp
    ${JOIPSources}/SceneScreen.h
    ${JOIPSources}/SceneScreen.ui
    ${JOIPSources}/Settings.cpp
    ${JOIPSources}/Settings.h
    ${JOIPSources}/SettingsScreen.cpp
    ${JOIPSources}/SettingsScreen.h
    ${JOIPSources}/SettingsScreen.ui
    ${JOIPSources}/Style.cpp
    ${JOIPSources}/Style.h
    ${JOIPSources}/Systems/BackActionHandler.cpp
    ${JOIPSources}/Systems/BackActionHandler.h
    ${JOIPSources}/Systems/DatabaseData.cpp
    ${JOIPSources}/Systems/DatabaseData.h
    ${JOIPSources}/Systems/DatabaseImageProvider.cpp
    ${JOIPSources}/Systems/DatabaseImageProvider.h
    ${JOIPSources}/Systems/DatabaseInterface/ProjectData.h
    ${JOIPSources}/Systems/DatabaseInterface/ResourceData.h
    ${JOIPSources}/Systems/DatabaseInterface/SaveData.h
    ${JOIPSources}/Systems/DatabaseInterface/SceneData.h
    ${JOIPSources}/Systems/DatabaseInterface/TagData.h
    ${JOIPSources}/Systems/DatabaseIO.cpp
    ${JOIPSources}/Systems/DatabaseIO.h
    ${JOIPSources}/Systems/DatabaseManager.cpp
    ${JOIPSources}/Systems/DatabaseManager.h
    ${JOIPSources}/Systems/DeviceManager.cpp
    ${JOIPSources}/Systems/DeviceManager.h
    ${JOIPSources}/Systems/Devices/DeviceResources.qrc
    ${JOIPSources}/Systems/Devices/DeviceSettings.cpp
    ${JOIPSources}/Systems/Devices/DeviceSettings.h
    ${JOIPSources}/Systems/Devices/IDevice.h
    ${JOIPSources}/Systems/Devices/IDeviceConnector.h
    ${JOIPSources}/Systems/DialogueTree.cpp
    ${JOIPSources}/Systems/DialogueTree.h
    ${JOIPSources}/Systems/DLJobs/IDownloadJob.h
    ${JOIPSources}/Systems/DLJobs/DownloadJobRegistry.h
    ${JOIPSources}/Systems/DLJobs/EosDownloadJob.cpp
    ${JOIPSources}/Systems/DLJobs/EosDownloadJob.h
    ${JOIPSources}/Systems/DLJobs/EosDownloadJobWidget.ui
    ${JOIPSources}/Systems/DLJobs/EosPagesToScenesTransformer.cpp
    ${JOIPSources}/Systems/DLJobs/EosPagesToScenesTransformer.h
    ${JOIPSources}/Systems/DLJobs/EosResourceLocator.cpp
    ${JOIPSources}/Systems/DLJobs/EosResourceLocator.h
    ${JOIPSources}/Systems/DLJobs/EosResources.qrc
    ${JOIPSources}/Systems/EOS/CommandEosAudioBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosAudioBase.h
    ${JOIPSources}/Systems/EOS/CommandEosChoiceBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosChoiceBase.h
    ${JOIPSources}/Systems/EOS/CommandEosDisableSceneBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosDisableSceneBase.h
    ${JOIPSources}/Systems/EOS/CommandEosEnableSceneBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosEnableSceneBase.h
    ${JOIPSources}/Systems/EOS/CommandEosEndBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosEndBase.h
    ${JOIPSources}/Systems/EOS/CommandEosEvalBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosEvalBase.h
    ${JOIPSources}/Systems/EOS/CommandEosGotoBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosGotoBase.h
    ${JOIPSources}/Systems/EOS/CommandEosIfBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosIfBase.h
    ${JOIPSources}/Systems/EOS/CommandEosImageBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosImageBase.h
    ${JOIPSources}/Systems/EOS/CommandEosNoopBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosNoopBase.h
    ${JOIPSources}/Systems/EOS/CommandEosNotificationCloseBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosNotificationCloseBase.h
    ${JOIPSources}/Systems/EOS/CommandEosNotificationCreateBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosNotificationCreateBase.h
    ${JOIPSources}/Systems/EOS/CommandEosPromptBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosPromptBase.h
    ${JOIPSources}/Systems/EOS/CommandEosSayBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosSayBase.h
    ${JOIPSources}/Systems/EOS/CommandEosTimerBase.cpp
    ${JOIPSources}/Systems/EOS/CommandEosTimerBase.h
    ${JOIPSources}/Systems/EOS/EosCommands.h
    ${JOIPSources}/Systems/EOS/EosHelpers.cpp
    ${JOIPSources}/Systems/EOS/EosHelpers.h
    ${JOIPSources}/Systems/HelpFactory.cpp
    ${JOIPSources}/Systems/HelpFactory.h
    ${JOIPSources}/Systems/ISerializable.h
    ${JOIPSources}/Systems/IRunnableJob.h
    ${JOIPSources}/Systems/Kink.cpp
    ${JOIPSources}/Systems/Kink.h
    ${JOIPSources}/Systems/Lockable.h
    ${JOIPSources}/Systems/MetronomeManager.cpp
    ${JOIPSources}/Systems/MetronomeManager.h
    ${JOIPSources}/Systems/NotificationSender.cpp
    ${JOIPSources}/Systems/NotificationSender.h
    ${JOIPSources}/Systems/OverlayManager.cpp
    ${JOIPSources}/Systems/OverlayManager.h
    ${JOIPSources}/Systems/PhysFs/PhysFsFileEngine.cpp
    ${JOIPSources}/Systems/PhysFs/PhysFsFileEngine.h
    ${JOIPSources}/Systems/PhysFs/PhysFsQtAVIntegration.cpp
    ${JOIPSources}/Systems/PhysFs/PhysFsQtAVIntegration.h
    ${JOIPSources}/Systems/Project.cpp
    ${JOIPSources}/Systems/Project.h
    ${JOIPSources}/Systems/ProjectDownloader.cpp
    ${JOIPSources}/Systems/ProjectDownloader.h
    ${JOIPSources}/Systems/ProjectJobWorker.cpp
    ${JOIPSources}/Systems/ProjectJobWorker.h
    ${JOIPSources}/Systems/Resource.cpp
    ${JOIPSources}/Systems/Resource.h
    ${JOIPSources}/Systems/ResourceBundle.cpp
    ${JOIPSources}/Systems/ResourceBundle.h
    ${JOIPSources}/Systems/SaveData.cpp
    ${JOIPSources}/Systems/SaveData.h
    ${JOIPSources}/Systems/Scene.cpp
    ${JOIPSources}/Systems/Scene.h
    ${JOIPSources}/Systems/Tag.cpp
    ${JOIPSources}/Systems/Tag.h
    ${JOIPSources}/Systems/JSON/JsonInstructionBase.h
    ${JOIPSources}/Systems/JSON/JsonInstructionNode.cpp
    ${JOIPSources}/Systems/JSON/JsonInstructionNode.h
    ${JOIPSources}/Systems/JSON/JsonInstructionSetParser.cpp
    ${JOIPSources}/Systems/JSON/JsonInstructionSetParser.h
    ${JOIPSources}/Systems/JSON/JsonInstructionSetRunner.h
    ${JOIPSources}/Systems/JSON/JsonInstructionSetRunnerItemModel.cpp
    ${JOIPSources}/Systems/JSON/JsonInstructionSetRunnerItemModel.h
    ${JOIPSources}/Systems/JSON/JsonInstructionSetSaxParser.h
    ${JOIPSources}/Systems/JSON/JsonInstructionTypes.h
    ${JOIPSources}/Systems/Script/CodeGeneratorProvider.cpp
    ${JOIPSources}/Systems/Script/CodeGeneratorProvider.h
    ${JOIPSources}/Systems/Script/CommonCodeGenerator.cpp
    ${JOIPSources}/Systems/Script/CommonCodeGenerator.h
    ${JOIPSources}/Systems/Script/CommonScriptHelpers.cpp
    ${JOIPSources}/Systems/Script/CommonScriptHelpers.h
    ${JOIPSources}/Systems/Script/EosScriptRunner.cpp
    ${JOIPSources}/Systems/Script/EosScriptRunner.h
    ${JOIPSources}/Systems/Script/ICodeGenerator.h
    ${JOIPSources}/Systems/Script/IScriptRunner.h
    ${JOIPSources}/Systems/Script/JsCodeGenerator.cpp
    ${JOIPSources}/Systems/Script/JsCodeGenerator.h
    ${JOIPSources}/Systems/Script/JsScriptRunner.cpp
    ${JOIPSources}/Systems/Script/JsScriptRunner.h
    ${JOIPSources}/Systems/Script/LuaCodeGenerator.cpp
    ${JOIPSources}/Systems/Script/LuaCodeGenerator.h
    ${JOIPSources}/Systems/Script/LuaScriptRunner.cpp
    ${JOIPSources}/Systems/Script/LuaScriptRunner.h
    ${JOIPSources}/Systems/Script/ScriptBackground.cpp
    ${JOIPSources}/Systems/Script/ScriptBackground.h
    ${JOIPSources}/Systems/Script/ScriptCacheFileEngine.cpp
    ${JOIPSources}/Systems/Script/ScriptCacheFileEngine.h
    ${JOIPSources}/Systems/Script/ScriptDbWrappers.cpp
    ${JOIPSources}/Systems/Script/ScriptDbWrappers.h
    ${JOIPSources}/Systems/Script/ScriptDeviceController.cpp
    ${JOIPSources}/Systems/Script/ScriptDeviceController.h
    ${JOIPSources}/Systems/Script/ScriptEval.cpp
    ${JOIPSources}/Systems/Script/ScriptEval.h
    ${JOIPSources}/Systems/Script/ScriptEventSender.cpp
    ${JOIPSources}/Systems/Script/ScriptEventSender.h
    ${JOIPSources}/Systems/Script/ScriptIcon.cpp
    ${JOIPSources}/Systems/Script/ScriptIcon.h
    ${JOIPSources}/Systems/Script/ScriptMediaPlayer.cpp
    ${JOIPSources}/Systems/Script/ScriptMediaPlayer.h
    ${JOIPSources}/Systems/Script/ScriptMetronome.cpp
    ${JOIPSources}/Systems/Script/ScriptMetronome.h
    ${JOIPSources}/Systems/Script/ScriptNotification.cpp
    ${JOIPSources}/Systems/Script/ScriptNotification.h
    ${JOIPSources}/Systems/Script/ScriptObjectBase.cpp
    ${JOIPSources}/Systems/Script/ScriptObjectBase.h
    ${JOIPSources}/Systems/Script/ScriptRunnerInstanceController.cpp
    ${JOIPSources}/Systems/Script/ScriptRunnerInstanceController.h
    ${JOIPSources}/Systems/Script/ScriptRunnerSignalEmiter.cpp
    ${JOIPSources}/Systems/Script/ScriptRunnerSignalEmiter.h
    ${JOIPSources}/Systems/Script/ScriptSceneManager.cpp
    ${JOIPSources}/Systems/Script/ScriptSceneManager.h
    ${JOIPSources}/Systems/Script/ScriptStorage.cpp
    ${JOIPSources}/Systems/Script/ScriptStorage.h
    ${JOIPSources}/Systems/Script/ScriptTextBox.cpp
    ${JOIPSources}/Systems/Script/ScriptTextBox.h
    ${JOIPSources}/Systems/Script/ScriptThread.cpp
    ${JOIPSources}/Systems/Script/ScriptThread.h
    ${JOIPSources}/Systems/Script/ScriptTimer.cpp
    ${JOIPSources}/Systems/Script/ScriptTimer.h
    ${JOIPSources}/Systems/Script/SequenceRunner.cpp
    ${JOIPSources}/Systems/Script/SequenceRunner.h
    ${JOIPSources}/Systems/ScriptRunner.cpp
    ${JOIPSources}/Systems/ScriptRunner.h
    ${JOIPSources}/Systems/Sequence/ISequenceObjectRunner.h
    ${JOIPSources}/Systems/Sequence/Sequence.cpp
    ${JOIPSources}/Systems/Sequence/Sequence.h
    ${JOIPSources}/Systems/Sequence/SequenceDeviceControllerRunner.cpp
    ${JOIPSources}/Systems/Sequence/SequenceDeviceControllerRunner.h
    ${JOIPSources}/Systems/Sequence/SequenceEvalRunner.cpp
    ${JOIPSources}/Systems/Sequence/SequenceEvalRunner.h
    ${JOIPSources}/Systems/Sequence/SequenceMediaPlayerRunner.cpp
    ${JOIPSources}/Systems/Sequence/SequenceMediaPlayerRunner.h
    ${JOIPSources}/Systems/Sequence/SequenceMetronomeRunner.cpp
    ${JOIPSources}/Systems/Sequence/SequenceMetronomeRunner.h
    ${JOIPSources}/Systems/Sequence/SequenceTextBoxRunner.cpp
    ${JOIPSources}/Systems/Sequence/SequenceTextBoxRunner.h
    ${JOIPSources}/Systems/ThreadedSystem.cpp
    ${JOIPSources}/Systems/ThreadedSystem.h
    ${JOIPSources}/Themes.cpp
    ${JOIPSources}/Themes.h
    ${JOIPSources}/UISoundEmitter.cpp
    ${JOIPSources}/UISoundEmitter.h
    ${JOIPSources}/Utils/MetronomeHelpers.cpp
    ${JOIPSources}/Utils/MetronomeHelpers.h
    ${JOIPSources}/Utils/MultiEmitterSoundPlayer.cpp
    ${JOIPSources}/Utils/MultiEmitterSoundPlayer.h
    ${JOIPSources}/Utils/RaiiFunctionCaller.cpp
    ${JOIPSources}/Utils/RaiiFunctionCaller.h
    ${JOIPSources}/Utils/ThreadUtils.cpp
    ${JOIPSources}/Utils/ThreadUtils.h
    ${JOIPSources}/Utils/UndoRedoFilter.cpp
    ${JOIPSources}/Utils/UndoRedoFilter.h
    ${JOIPSources}/Utils/WidgetHelpers.cpp
    ${JOIPSources}/Utils/WidgetHelpers.h
    ${JOIPSources}/Widgets/AgeCheckOverlay.cpp
    ${JOIPSources}/Widgets/AgeCheckOverlay.h
    ${JOIPSources}/Widgets/AgeCheckOverlay.ui
    ${JOIPSources}/Widgets/BackgroundWidget.cpp
    ${JOIPSources}/Widgets/BackgroundWidget.h
    ${JOIPSources}/Widgets/ColorPicker.cpp
    ${JOIPSources}/Widgets/ColorPicker.h
    ${JOIPSources}/Widgets/ColorPicker.ui
    ${JOIPSources}/Widgets/DeviceButtonOverlay.cpp
    ${JOIPSources}/Widgets/DeviceButtonOverlay.h
    ${JOIPSources}/Widgets/DeviceSelectorWidget.cpp
    ${JOIPSources}/Widgets/DeviceSelectorWidget.h
    ${JOIPSources}/Widgets/DeviceSelectorWidget.ui
    ${JOIPSources}/Widgets/DownloadButtonOverlay.cpp
    ${JOIPSources}/Widgets/DownloadButtonOverlay.h
    ${JOIPSources}/Widgets/Editor/EditorCustomBlockUserData.cpp
    ${JOIPSources}/Widgets/Editor/EditorCustomBlockUserData.h
    ${JOIPSources}/Widgets/Editor/EditorHighlighter.cpp
    ${JOIPSources}/Widgets/Editor/EditorHighlighter.h
    ${JOIPSources}/Widgets/Editor/EditorSearchBar.cpp
    ${JOIPSources}/Widgets/Editor/EditorSearchBar.h
    ${JOIPSources}/Widgets/Editor/HighlightedSearchableTextEdit.cpp
    ${JOIPSources}/Widgets/Editor/HighlightedSearchableTextEdit.h
    ${JOIPSources}/Widgets/Editor/RichTextEdit.cpp
    ${JOIPSources}/Widgets/Editor/RichTextEdit.h
    ${JOIPSources}/Widgets/Editor/TextEditZoomEnabler.cpp
    ${JOIPSources}/Widgets/Editor/TextEditZoomEnabler.h
    ${JOIPSources}/Widgets/FlowLayout.cpp
    ${JOIPSources}/Widgets/FlowLayout.h
    ${JOIPSources}/Widgets/HelpOverlay.cpp
    ${JOIPSources}/Widgets/HelpOverlay.h
    ${JOIPSources}/Widgets/HelpOverlay.ui
    ${JOIPSources}/Widgets/HtmlViewDelegate.cpp
    ${JOIPSources}/Widgets/HtmlViewDelegate.h
    ${JOIPSources}/Widgets/IWidgetBaseInterface.h
    ${JOIPSources}/Widgets/LongLongSpinBox.cpp
    ${JOIPSources}/Widgets/LongLongSpinBox.h
    ${JOIPSources}/Widgets/MenuButton.cpp
    ${JOIPSources}/Widgets/MenuButton.h
    ${JOIPSources}/Widgets/MinimizingScrollArea.cpp
    ${JOIPSources}/Widgets/MinimizingScrollArea.h
    ${JOIPSources}/Widgets/OverlayBase.cpp
    ${JOIPSources}/Widgets/OverlayBase.h
    ${JOIPSources}/Widgets/OverlayButton.cpp
    ${JOIPSources}/Widgets/OverlayButton.h
    ${JOIPSources}/Widgets/Player/AvSlider.cpp
    ${JOIPSources}/Widgets/Player/AvSlider.h
    ${JOIPSources}/Widgets/Player/MediaPlayer.cpp
    ${JOIPSources}/Widgets/Player/MediaPlayer.h
    ${JOIPSources}/Widgets/PositionalMenu.cpp
    ${JOIPSources}/Widgets/PositionalMenu.h
    ${JOIPSources}/Widgets/ProgressBar.cpp
    ${JOIPSources}/Widgets/ProgressBar.h
    ${JOIPSources}/Widgets/ProjectCardSelectionWidget.cpp
    ${JOIPSources}/Widgets/ProjectCardSelectionWidget.h
    ${JOIPSources}/Widgets/ProjectCardSelectionWidget.ui
    ${JOIPSources}/Widgets/PushNotification.cpp
    ${JOIPSources}/Widgets/PushNotification.h
    ${JOIPSources}/Widgets/ResourceDisplayWidget.cpp
    ${JOIPSources}/Widgets/ResourceDisplayWidget.h
    ${JOIPSources}/Widgets/ResourceDisplayWidget.ui
    ${JOIPSources}/Widgets/SearchWidget.cpp
    ${JOIPSources}/Widgets/SearchWidget.h
    ${JOIPSources}/Widgets/SearchWidget.ui
    ${JOIPSources}/Widgets/ShortcutButton.cpp
    ${JOIPSources}/Widgets/ShortcutButton.h
    ${JOIPSources}/Widgets/SlidingStackedWidget.cpp
    ${JOIPSources}/Widgets/SlidingStackedWidget.h
    ${JOIPSources}/Widgets/SlidingTabWidget.cpp
    ${JOIPSources}/Widgets/SlidingTabWidget.h
    ${JOIPSources}/Widgets/SlidingWidget.cpp
    ${JOIPSources}/Widgets/SlidingWidget.h
    ${JOIPSources}/Widgets/TagCompleter.cpp
    ${JOIPSources}/Widgets/TagCompleter.h
    ${JOIPSources}/Widgets/TagsView.cpp
    ${JOIPSources}/Widgets/TagsView.h
    ${JOIPSources}/Widgets/TagsView.ui
    ${JOIPSources}/Widgets/TitleLabel.cpp
    ${JOIPSources}/Widgets/TitleLabel.h
    ${JOIPSources}/Widgets/ZoomComboBox.cpp
    ${JOIPSources}/Widgets/ZoomComboBox.h
    ${JOIPSources}/WindowContext.cpp
    ${JOIPSources}/WindowContext.h
    ${JOIPSources}/main.cpp
    ${CMAKE_SOURCE_DIR}/resources.qrc
    ${CMAKE_SOURCE_DIR}/version.cpp
    ${CMAKE_SOURCE_DIR}/version.h
  )

  #platform sources
  if (WIN32)
    get_target_property(WINTOAST_SRC WinToast SOURCES)
    set(WinSources
      ${JOIPSources}/Windows/WindowsMainWindow.cpp
      ${JOIPSources}/Windows/WindowsMainWindow.h
      ${JOIPSources}/Windows/WindowsNativePushNotification.cpp
      ${JOIPSources}/Windows/WindowsNativePushNotification.h
      ${WINTOAST_SRC}
    )
  elseif(ANDROID)
    set(AndroidSources
      ${JOIPSources}/Android/AndroidApplicationWindow.cpp
      ${JOIPSources}/Android/AndroidApplicationWindow.h
      ${JOIPSources}/Android/AndroidMainWindow.cpp
      ${JOIPSources}/Android/AndroidMainWindow.h
      ${JOIPSources}/Android/AndroidNavigationBar.cpp
      ${JOIPSources}/Android/AndroidNavigationBar.h
      ${JOIPSources}/Android/AndroidNotificationClient.cpp
      ${JOIPSources}/Android/AndroidNotificationClient.h
      ${CMAKE_SOURCE_DIR}/android_resources.qrc
    )
  elseif(LINUX)
    set(LinuxSources
      ${JOIPSources}/Linux/LinuxMainWindow.cpp
      ${JOIPSources}/Linux/LinuxMainWindow.h
    )
  endif()

  #aditional conditional sources
  if (HAS_BUTTPLUG_CPP)
    set(OptionalSources
      ${OptionalSources}
      ${JOIPSources}/Systems/Devices/ButtplugDeviceConnector.cpp
      ${JOIPSources}/Systems/Devices/ButtplugDeviceConnector.h
      ${JOIPSources}/Systems/Devices/ButtplugDeviceConnectorContext.cpp
      ${JOIPSources}/Systems/Devices/ButtplugDeviceConnectorContext.h
      ${JOIPSources}/Systems/Devices/ButtplugDeviceWrapper.cpp
      ${JOIPSources}/Systems/Devices/ButtplugDeviceWrapper.h
    )
  endif()

  set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${JOIPSources}/Editor)

  if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    if(ANDROID)
      qt_add_executable(${JOIP_PROJECT_NAME}
          MANUAL_FINALIZATION
          ${Sources}
          ${AndroidSources}
          ${OptionalSources}
          ${CMAKE_SOURCE_DIR}/android_resources.qrc
      )
    elseif(WIN32)
      qt_add_executable(${JOIP_PROJECT_NAME}
          MANUAL_FINALIZATION
          ${Sources}
          ${WinSources}
          ${OptionalSources}
      )
    elseif(LINUX)
      qt_add_executable(${JOIP_PROJECT_NAME}
          MANUAL_FINALIZATION
          ${Sources}
          ${LinuxSources}
          ${OptionalSources}
      )
      target_compile_options(${JOIP_PROJECT_NAME} PUBLIC -fPIC)
    else()
      qt_add_executable(${JOIP_PROJECT_NAME}
          MANUAL_FINALIZATION
          ${Sources}
      )
    endif()
  # Define target properties for Android with Qt 6 as:
  #    set_property(TARGET Test APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
  #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
  # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
  else()
      if(ANDROID)
          add_library(${JOIP_PROJECT_NAME} SHARED
              ${Sources}
              ${AndroidSources}
              ${OptionalSources}
          )
  # Define properties for Android with Qt 5 after find_package() calls as:
  #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
      elseif(WIN32)
          add_executable(${JOIP_PROJECT_NAME}
              ${Sources}
              ${WinSources}
              ${OptionalSources}
          )
      elseif(LINUX)
        add_executable(${JOIP_PROJECT_NAME}
            ${Sources}
            ${LinuxSources}
            ${OptionalSources}
        )
        target_compile_options(${JOIP_PROJECT_NAME} PUBLIC -fPIC)
      else()
          add_executable(${JOIP_PROJECT_NAME}
              ${Sources}
              ${OptionalSources}
          )
      endif()
  endif()

  target_compile_definitions(${JOIP_PROJECT_NAME} PRIVATE QT_FEATURE_zstd=-1)
endmacro()

#---------------------------------------------------------------------------------------
# Libs & Project Settings
#---------------------------------------------------------------------------------------
macro(JOIPProjectSettings JOIP_PROJECT_NAME)
  target_link_libraries(${JOIP_PROJECT_NAME}
    PRIVATE
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Multimedia
      Qt${QT_VERSION_MAJOR}::MultimediaWidgets
      Qt${QT_VERSION_MAJOR}::Network
      Qt${QT_VERSION_MAJOR}::Qml
      Qt${QT_VERSION_MAJOR}::Quick
      Qt${QT_VERSION_MAJOR}::QuickControls2
      Qt${QT_VERSION_MAJOR}::QuickWidgets
      Qt${QT_VERSION_MAJOR}::Svg
      Qt${QT_VERSION_MAJOR}::Widgets
      #Qt${QT_VERSION_MAJOR}::WidgetsPrivate
      Qt${QT_VERSION_MAJOR}::WebChannel
      Qt${QT_VERSION_MAJOR}::Xml
      Qt::AV
      Qt::AVWidgets
      Qt::Rcc_static
      NodeEditor::nodes
      KF5SyntaxHighlighting
      better_enums
      nlohmann_json_schema_validator
      physfs-static
      xmldom
      qtlua
      json_lua
      lua_sandbox
      zip
    )

    if (HAS_BUTTPLUG_CPP)
      target_link_libraries(${JOIP_PROJECT_NAME}
        PRIVATE
          Qt${QT_VERSION_MAJOR}Buttplug)
      target_compile_definitions(${JOIP_PROJECT_NAME}
        PRIVATE
          -DHAS_BUTTPLUG_CPP=1)
    endif()

    if (KDE_DEBUG)
      target_link_libraries(${JOIP_PROJECT_NAME}
        PRIVATE
          Qt${QT_VERSION_MAJOR}::Test)
    endif()

    if (WIN32)
      find_library(PSAPI NAMES "psapi" REQUIRED)
      message (STATUS "PSAPI: ${PSAPI}")
      target_link_libraries(${JOIP_PROJECT_NAME}
        PRIVATE
          Qt${QT_VERSION_MAJOR}::WinExtras
          WinToast
          Psapi)
    endif()

  target_sources(${JOIP_PROJECT_NAME}
    PRIVATE
      $<TARGET_OBJECTS:MRichTextEditor>
      $<TARGET_OBJECTS:SortFilterProxyModel>)

  target_include_directories(${JOIP_PROJECT_NAME}
    PRIVATE
      ${JOIPSources}
      ${Qt${QT_VERSION_MAJOR}Core_PRIVATE_INCLUDE_DIRS}
      ${Qt${QT_VERSION_MAJOR}Gui_PRIVATE_INCLUDE_DIRS}
      ${Qt${QT_VERSION_MAJOR}Widgets_PRIVATE_INCLUDE_DIRS}
      ${FFMPEG_INCLUDE_DIRS}
      ${NodeEditor_INCLUDE_DIRS}
      ${PhysicsFS_INCLUDE_DIRS}
      $<TARGET_PROPERTY:MRichTextEditor,INTERFACE_INCLUDE_DIRECTORIES>
      $<TARGET_PROPERTY:SortFilterProxyModel,INTERFACE_INCLUDE_DIRECTORIES>
      $<TARGET_PROPERTY:xmldom,INTERFACE_INCLUDE_DIRECTORIES>
      $<TARGET_PROPERTY:json_lua,INTERFACE_INCLUDE_DIRECTORIES>
    )

  target_compile_definitions(${JOIP_PROJECT_NAME}
    PRIVATE
      $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
      $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_DECLARATIVE_DEBUG>
      ${NodeEditor_DEFINITIONS})

  #-----------------------------------------------------------------------------------------
  # Android stuff
  #-----------------------------------------------------------------------------------------
  if (ANDROID)
    target_link_libraries(${JOIP_PROJECT_NAME} PRIVATE Qt5::AndroidExtras)

    set_target_properties(${JOIP_PROJECT_NAME} PROPERTIES
        QT_ANDROID_TARGET_SDK_VERSION 31
        QT_ANDROID_SDK_BUILD_TOOLS_REVISION 31.0.0)

    set(ANDROID_PERMISSIONS
      "android.permission.INTERNET"
      "android.permission.WRITE_EXTERNAL_STORAGE"
      "android.permission.WRITE_SETTINGS")

    set(ANDROID_FEATURES
      "android.hardware.opengles.aep")
  endif()

  #-----------------------------------------------------------------------------------------
  # Windows stuff
  #-----------------------------------------------------------------------------------------
  if (WIN32)
    foreach(win_extra_lib ${CUSTOM_DESKTOP_EXTRA_LIBS})
      add_custom_command(
              TARGET ${JOIP_PROJECT_NAME} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
                      ${win_extra_lib}
                      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endforeach()
  endif()

  #-----------------------------------------------------------------------------------------
  # Linux stuff
  #-----------------------------------------------------------------------------------------
  if (LINUX)
    set_property(TARGET ${JOIP_PROJECT_NAME}  APPEND_STRING PROPERTY
      LINK_FLAGS " -Wl,-rpath-link,${ffmpeg_dir}")
  endif()

  #-----------------------------------------------------------------------------------------
  # Version and icon
  #-----------------------------------------------------------------------------------------
  set_target_properties(${JOIP_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${JOIP_PROJECT_NAME})
  set(APPNAME ${JOIP_PROJECT_NAME})
  set(SETICONPATH ON)
  set(ICONPATH "${CMAKE_SOURCE_DIR}/resources/img/Icon.ico")

  set(${JOIP_PROJECT_NAME}_VERSION_MAJOR ${JOIP_VERSION_MAJOR})
  set(${JOIP_PROJECT_NAME}_VERSION_MINOR ${JOIP_VERSION_MINOR})
  set(${JOIP_PROJECT_NAME}_VERSION_PATCH ${JOIP_VERSION_PATCH})

  set(${JOIP_PROJECT_NAME}_VERSION "${${JOIP_PROJECT_NAME}_VERSION_MAJOR}.${${JOIP_PROJECT_NAME}_VERSION_MINOR}.${${JOIP_PROJECT_NAME}_VERSION_PATCH}")
  set(VERSION_DOT ${${JOIP_PROJECT_NAME}_VERSION_MAJOR}.${${JOIP_PROJECT_NAME}_VERSION_MINOR}.${${JOIP_PROJECT_NAME}_VERSION_PATCH} CACHE INTERNAL "")
  set(VERSION_COMMA ${${JOIP_PROJECT_NAME}_VERSION_MAJOR},${${JOIP_PROJECT_NAME}_VERSION_MINOR},${${JOIP_PROJECT_NAME}_VERSION_PATCH} CACHE INTERNAL "")

  set_target_properties(${JOIP_PROJECT_NAME} PROPERTIES VERSION "${${JOIP_PROJECT_NAME}_VERSION}")

  set(PROJECT_VERSION ${${JOIP_PROJECT_NAME}_VERSION})

  get_target_property(OUTPUT_BASENAME ${JOIP_PROJECT_NAME} OUTPUT_NAME)
  set(OUTPUT_FILENAME "$<TARGET_FILE_NAME:${JOIP_PROJECT_NAME}>")

  if (WIN32)
    target_sources(${JOIP_PROJECT_NAME}
      PRIVATE
      "${CMAKE_SOURCE_DIR}/version.cpp"
      "${CMAKE_SOURCE_DIR}/version.h.in"
      "${CMAKE_SOURCE_DIR}/version_win.rc.in"
      "${PROJECT_BINARY_DIR}/version_win.rc"
      )

    configure_file(
      "${CMAKE_SOURCE_DIR}/version_win.rc.in"
      "${PROJECT_BINARY_DIR}/version_win.rc"
    )
  else()
    target_sources(${JOIP_PROJECT_NAME}
      PRIVATE
      "${CMAKE_SOURCE_DIR}/version.cpp"
      "${CMAKE_SOURCE_DIR}/version.h.in"
      )
  endif()

  configure_file_generate(
    "${CMAKE_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/out_Version.h"
  )

endmacro(JOIPProjectSettings)

#-----------------------------------------------------------------------------------------
# Finalization under Qt6
#-----------------------------------------------------------------------------------------
macro(FinalizeJOIPProject JOIP_PROJECT_NAME)
  if(QT_VERSION_MAJOR EQUAL 6)
      qt_finalize_executable(${JOIP_PROJECT_NAME})
  endif()
endmacro(FinalizeJOIPProject)
