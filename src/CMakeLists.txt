cmake_minimum_required(VERSION 3.5)

project(JOIPEngine LANGUAGES CXX)

include(JOIPCommon)
include(QtAV/FindQtAV)

find_package(Qt5 COMPONENTS
    Core
    Multimedia
    MultimediaWidgets
    Network
    OpenGL
    Qml
    Quick
    QuickControls2
    QuickWidgets
    Svg
    Widgets
    WebChannel
    Xml
  REQUIRED)

set(Sources
  Application.cpp 
  Application.h 
  Constants.h 
  CreditsScreen.cpp 
  CreditsScreen.h 
  CreditsScreen.ui 
  Editor/EditorActionBar.cpp 
  Editor/EditorActionBar.h 
  Editor/EditorActionBar.ui 
  Editor/EditorChoiceScreen.cpp 
  Editor/EditorChoiceScreen.h 
  Editor/EditorChoiceScreen.ui 
  Editor/EditorCodeWidget.cpp 
  Editor/EditorCodeWidget.h 
  Editor/EditorCodeWidget.ui 
  Editor/EditorMainScreen.cpp 
  Editor/EditorMainScreen.h 
  Editor/EditorMainScreen.ui 
  Editor/EditorModel.cpp 
  Editor/EditorModel.h 
  Editor/EditorProjectSettingsWidget.cpp 
  Editor/EditorProjectSettingsWidget.h 
  Editor/EditorProjectSettingsWidget.ui 
  Editor/EditorResourceDisplayWidget.cpp 
  Editor/EditorResourceDisplayWidget.h 
  Editor/EditorResourceDisplayWidget.ui 
  Editor/EditorResourceWidget.cpp 
  Editor/EditorResourceWidget.h 
  Editor/EditorResourceWidget.ui 
  Editor/EditorSceneNodeWidget.cpp 
  Editor/EditorSceneNodeWidget.h 
  Editor/EditorSceneNodeWidget.ui 
  Editor/EditorWidgetBase.cpp 
  Editor/EditorWidgetBase.h 
  Editor/EditorWidgetTypes.h 
  Editor/NodeEditor/EndNodeModel.cpp 
  Editor/NodeEditor/EndNodeModel.h 
  Editor/NodeEditor/FlowView.cpp 
  Editor/NodeEditor/FlowView.h 
  Editor/NodeEditor/PathMergerModel.cpp 
  Editor/NodeEditor/PathMergerModel.h 
  Editor/NodeEditor/PathSplitterModel.cpp 
  Editor/NodeEditor/PathSplitterModel.h 
  Editor/NodeEditor/PathSplitterModelWidget.cpp 
  Editor/NodeEditor/PathSplitterModelWidget.h 
  Editor/NodeEditor/PathSplitterModelWidget.ui 
  Editor/NodeEditor/SceneNodeModel.cpp 
  Editor/NodeEditor/SceneNodeModel.h 
  Editor/NodeEditor/SceneNodeModelWidget.cpp 
  Editor/NodeEditor/SceneNodeModelWidget.h 
  Editor/NodeEditor/SceneNodeModelWidget.ui 
  Editor/NodeEditor/SceneTranstitionData.cpp 
  Editor/NodeEditor/SceneTranstitionData.h 
  Editor/NodeEditor/StartNodeModel.cpp 
  Editor/NodeEditor/StartNodeModel.h 
  Editor/Project/KinkCompleter.cpp 
  Editor/Project/KinkCompleter.h 
  Editor/Project/KinkSelectionOverlay.cpp 
  Editor/Project/KinkSelectionOverlay.h 
  Editor/Project/KinkSelectionOverlay.ui
  Editor/Project/KinkTreeItem.cpp 
  Editor/Project/KinkTreeItem.h 
  Editor/Project/KinkTreeModel.cpp 
  Editor/Project/KinkTreeModel.h 
  Editor/Project/KinkTreeSortFilterProxyModel.cpp
  Editor/Project/KinkTreeSortFilterProxyModel.h
  Editor/Resources/ResourceTreeItem.cpp 
  Editor/Resources/ResourceTreeItem.h 
  Editor/Resources/ResourceTreeItemModel.cpp 
  Editor/Resources/ResourceTreeItemModel.h 
  Editor/Resources/ResourceTreeItemSortFilterProxyModel.cpp 
  Editor/Resources/ResourceTreeItemSortFilterProxyModel.h 
  Editor/Resources/WebResourceOverlay.cpp 
  Editor/Resources/WebResourceOverlay.h 
  Editor/Resources/WebResourceOverlay.ui 
  Editor/Script/BackgroundSnippetOverlay.cpp 
  Editor/Script/BackgroundSnippetOverlay.h 
  Editor/Script/BackgroundSnippetOverlay.ui 
  Editor/Script/IconSnippetOverlay.cpp 
  Editor/Script/IconSnippetOverlay.h 
  Editor/Script/IconSnippetOverlay.ui 
  Editor/Script/ResourceSnippetOverlay.cpp 
  Editor/Script/ResourceSnippetOverlay.h 
  Editor/Script/ResourceSnippetOverlay.ui 
  Editor/Script/ScriptEditorModel.cpp 
  Editor/Script/ScriptEditorModel.h 
  Editor/Script/ScriptEditorWidget.cpp 
  Editor/Script/ScriptEditorWidget.h 
  Editor/Script/ScriptHighlighter.cpp 
  Editor/Script/ScriptHighlighter.h 
  Editor/Script/TextSnippetOverlay.cpp 
  Editor/Script/TextSnippetOverlay.h 
  Editor/Script/TextSnippetOverlay.ui 
  Editor/Script/ThreadSnippetOverlay.cpp 
  Editor/Script/ThreadSnippetOverlay.h 
  Editor/Script/ThreadSnippetOverlay.ui 
  Editor/Script/TimerSnippetOverlay.cpp 
  Editor/Script/TimerSnippetOverlay.h 
  Editor/Script/TimerSnippetOverlay.ui 
  EditorScreen.cpp 
  EditorScreen.h 
  EditorScreen.ui 
  Enums.h 
  IAppStateScreen.h 
  MainScreen.cpp 
  MainScreen.h 
  MainScreen.ui 
  MainWindow.cpp 
  MainWindow.h 
  MainWindow.ui 
  Player/InformationWidget.cpp 
  Player/InformationWidget.h 
  Player/InformationWidget.ui 
  Player/ProjectRunner.cpp 
  Player/ProjectRunner.h 
  Player/SceneMainScreen.cpp 
  Player/SceneMainScreen.h 
  Player/SceneMainScreen.ui 
  Player/TextBoxWidget.cpp 
  Player/TextBoxWidget.h 
  Player/TextBoxWidget.ui 
  Player/TimerDisplayWidget.cpp 
  Player/TimerDisplayWidget.h 
  Player/TimerDisplayWidget.ui 
  Player/TimerWidget.cpp 
  Player/TimerWidget.h 
  SVersion.h 
  SceneScreen.cpp 
  SceneScreen.h 
  SceneScreen.ui 
  Settings.cpp 
  Settings.h 
  SettingsScreen.cpp 
  SettingsScreen.h 
  SettingsScreen.ui 
  Style.cpp 
  Style.h 
  Systems/DatabaseImageProvider.cpp 
  Systems/DatabaseImageProvider.h 
  Systems/DatabaseManager.cpp 
  Systems/DatabaseManager.h 
  Systems/HelpFactory.cpp 
  Systems/HelpFactory.h 
  Systems/ISerializable.h 
  Systems/Kink.cpp 
  Systems/Kink.h 
  Systems/OverlayManager.cpp 
  Systems/OverlayManager.h 
  Systems/Project.cpp 
  Systems/Project.h 
  Systems/Resource.cpp 
  Systems/Resource.h 
  Systems/Scene.cpp 
  Systems/Scene.h 
  Systems/Script/ScriptBackground.cpp 
  Systems/Script/ScriptBackground.h 
  Systems/Script/ScriptIcon.cpp 
  Systems/Script/ScriptIcon.h 
  Systems/Script/ScriptMediaPlayer.cpp 
  Systems/Script/ScriptMediaPlayer.h 
  Systems/Script/ScriptObjectBase.cpp 
  Systems/Script/ScriptObjectBase.h 
  Systems/Script/ScriptRunnerSignalEmiter.cpp 
  Systems/Script/ScriptRunnerSignalEmiter.h 
  Systems/Script/ScriptStorage.cpp 
  Systems/Script/ScriptStorage.h 
  Systems/Script/ScriptTextBox.cpp 
  Systems/Script/ScriptTextBox.h 
  Systems/Script/ScriptThread.cpp 
  Systems/Script/ScriptThread.h 
  Systems/Script/ScriptTimer.cpp 
  Systems/Script/ScriptTimer.h 
  Systems/ScriptRunner.cpp 
  Systems/ScriptRunner.h 
  Systems/ThreadedSystem.cpp 
  Systems/ThreadedSystem.h 
  UISoundEmitter.cpp 
  UISoundEmitter.h 
  Widgets/BackgroundWidget.cpp 
  Widgets/BackgroundWidget.h 
  Widgets/ColorPicker.cpp 
  Widgets/ColorPicker.h 
  Widgets/ColorPicker.ui 
  Widgets/FlowLayout.cpp 
  Widgets/FlowLayout.h 
  Widgets/HelpOverlay.cpp 
  Widgets/HelpOverlay.h 
  Widgets/HelpOverlay.ui 
  Widgets/IWidgetBaseInterface.h 
  Widgets/MenuButton.cpp 
  Widgets/MenuButton.h 
  Widgets/OverlayBase.cpp 
  Widgets/OverlayBase.h 
  Widgets/Player/MediaPlayer.cpp 
  Widgets/Player/MediaPlayer.h   
  Widgets/ProjectCardSelectionWidget.cpp 
  Widgets/ProjectCardSelectionWidget.h 
  Widgets/ProjectCardSelectionWidget.ui 
  Widgets/ResourceDisplayWidget.cpp 
  Widgets/ResourceDisplayWidget.h 
  Widgets/ResourceDisplayWidget.ui 
  Widgets/SearchWidget.cpp 
  Widgets/SearchWidget.h 
  Widgets/SearchWidget.ui 
  Widgets/ShortcutButton.cpp 
  Widgets/ShortcutButton.h 
  Widgets/TitleLabel.cpp 
  Widgets/TitleLabel.h 
  WindowContext.cpp 
  WindowContext.h 
  main.cpp 
  ../resources.qrc
  ../version.cpp
  ../version.h
)

if(ANDROID)
  add_library(${PROJECT_NAME} SHARED ${Sources})
else()
  add_executable(${PROJECT_NAME} ${Sources})
endif()

#-----------------------------------------------------------------------------------------
# Libs
#-----------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Qt5::Core
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    Qt5::Network
    Qt5::OpenGL
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::QuickWidgets
    Qt5::Svg
    Qt5::Widgets
    Qt5::WebChannel
    Qt5::Xml
    ${QTAVWIDGETS_LIBRARIES}
    NodeEditor::nodes
    KF5SyntaxHighlighting
    better_enums
    physfs-static)

target_sources(${PROJECT_NAME}
  PRIVATE
    $<TARGET_OBJECTS:SortFilterProxyModel>)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${QTAVWIDGETS_INCLUDE_DIRS}
    ${NodeEditor_INCLUDE_DIRS}
    ${PhysicsFS_INCLUDE_DIRS}
    $<TARGET_PROPERTY:SortFilterProxyModel,INTERFACE_INCLUDE_DIRECTORIES>)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    ${NodeEditor_DEFINITIONS})

#-----------------------------------------------------------------------------------------
# Version and icon
#-----------------------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set(APPNAME ${PROJECT_NAME})
set(SETICONPATH ON)
set(ICONPATH "${CMAKE_SOURCE_DIR}/resources/img/Icon.ico")

set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MINOR 0)
set(${PROJECT_NAME}_VERSION_PATCH 1)

set(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")
set(VERSION_DOT ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH} CACHE INTERNAL "")
set(VERSION_COMMA ${${PROJECT_NAME}_VERSION_MAJOR},${${PROJECT_NAME}_VERSION_MINOR},${${PROJECT_NAME}_VERSION_PATCH} CACHE INTERNAL "")

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${${PROJECT_NAME}_VERSION}")

set(PROJECT_VERSION ${${PROJECT_NAME}_VERSION})

get_target_property(OUTPUT_BASENAME ${PROJECT_NAME} OUTPUT_NAME)
set(OUTPUT_FILENAME "$<TARGET_FILE_NAME:${PROJECT_NAME}>")

target_sources(${PROJECT_NAME}
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

configure_file_generate(
  "${CMAKE_SOURCE_DIR}/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/out_Version.h"
)