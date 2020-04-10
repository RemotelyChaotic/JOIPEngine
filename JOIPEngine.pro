#-------------------------------------------------
#
# Project created by QtCreator 2019-07-12T20:22:42
#
#-------------------------------------------------

QT += \
    core \
    gui \
    multimedia \
    multimediawidgets \
    network \
    opengl \
    qml \
    svg \
    webengine \
    webenginewidgets \
    xml

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets avwidgets
} else {
  CONFIG += avwidgets
}

TARGET = JOIPEngine
TEMPLATE = app

CONFIG(release, debug|release) {
  QMAKE_CXXFLAGS_RELEASE += /Zi
  QMAKE_LFLAGS_RELEASE += /DEBUG
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

include(lib/better-enums/_lib_enum.pri)
#CONFIG(debug, debug|release) {
#   include(lib/Fluid_Studios_Memory_Manager/_lib_mmgr.pri)
#}
CONFIG(debug, debug|release) {
  include (lib/modeltest/modeltest.pri)
}
include (lib/node/node.pri)

INCLUDEPATH += \
    src

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/Settings.cpp \
    src/Application.cpp \
    src/MainScreen.cpp \
    src/SettingsScreen.cpp \
    src/WindowContext.cpp \
    src/EditorScreen.cpp \
    src/Systems/ThreadedSystem.cpp \
    src/Systems/DatabaseManager.cpp \
    src/Systems/Project.cpp \
    src/Systems/Scene.cpp \
    src/Systems/Resource.cpp \
    src/Editor/EditorChoiceScreen.cpp \
    src/Editor/EditorMainScreen.cpp \
    src/Editor/Resources/ResourceTreeItem.cpp \
    src/Editor/Resources/ResourceTreeItemModel.cpp \
    src/Editor/EditorResourceWidget.cpp \
    src/Widgets/ProjectCardSelectionWidget.cpp \
    src/Widgets/OverlayBase.cpp \
    src/Editor/Resources/WebResourceOverlay.cpp \
    src/Widgets/Player/MediaPlayer.cpp \
    src/Widgets/ResourceDisplayWidget.cpp \
    src/Editor/EditorActionBar.cpp \
    src/Editor/EditorWidgetBase.cpp \
    src/Editor/EditorResourceDisplayWidget.cpp \
    src/Editor/Resources/ResourceTreeItemSortFilterProxyModel.cpp \
    src/Editor/EditorSceneNodeWidget.cpp \
    src/Editor/NodeEditor/SceneNodeModel.cpp \
    src/Editor/NodeEditor/SceneTranstitionData.cpp \
    src/Editor/NodeEditor/SceneNodeModelWidget.cpp \
    src/Editor/NodeEditor/StartNodeModel.cpp \
    src/Editor/NodeEditor/EndNodeModel.cpp \
    src/Editor/NodeEditor/FlowView.cpp \
    src/Editor/NodeEditor/PathMergerModel.cpp \
    src/Editor/NodeEditor/PathSplitterModel.cpp \
    src/Editor/NodeEditor/PathSplitterModelWidget.cpp \
    src/Systems/ScriptRunner.cpp \
    src/Editor/EditorCodeWidget.cpp \
    src/Editor/Script/ScriptEditorWidget.cpp \
    src/Editor/Script/ScriptHighlighter.cpp \
    src/Systems/ScriptBackground.cpp \
    src/Systems/ScriptMediaPlayer.cpp \
    src/Systems/ScriptTextBox.cpp \
    src/Systems/ScriptTimer.cpp \
    src/Systems/ScriptThread.cpp \
    src/Systems/ScriptIcon.cpp \
    src/SceneScreen.cpp \
    src/Player/SceneMainScreen.cpp \
    src/Player/TextBoxWidget.cpp \
    src/Player/InformationWidget.cpp \
    src/Player/ProjectRunner.cpp \
    src/Systems/ScriptRunnerSignalEmiter.cpp \
    src/Player/FlowLayout.cpp \
    src/Player/TimerDisplayWidget.cpp \
    src/Player/TimerWidget.cpp \
    src/Player/BackgroundWidget.cpp \
    src/Style.cpp \
    src/Widgets/MenuButton.cpp \
    src/Widgets/TitleLabel.cpp \
    src/Editor/Script/ResourceSnippetOverlay.cpp \
    src/Editor/Script/IconSnippetOverlay.cpp \
    src/Editor/Script/BackgroundSnippetOverlay.cpp \
    src/Editor/Script/TextSnippetOverlay.cpp \
    src/Editor/Script/TimerSnippetOverlay.cpp \
    src/Widgets/ColorPicker.cpp \
    src/CreditsScreen.cpp \
    src/Widgets/SearchWidget.cpp \
    src/Systems/ScriptStorage.cpp \
    src/UISoundEmitter.cpp \
    src/Editor/Script/ThreadSnippetOverlay.cpp \
    src/Editor/EditorModel.cpp \
    src/Editor/Script/ScriptEditorModel.cpp \
    src/Widgets/HelpOverlay.cpp \
    src/Systems/HelpFactory.cpp \
    src/Systems/OverlayManager.cpp \
    src/Editor/EditorProjectSettingsWidget.cpp

HEADERS += \
    src/MainWindow.h \
    src/Settings.h \
    src/Application.h \
    src/MainScreen.h \
    src/SettingsScreen.h \
    src/Enums.h \
    src/WindowContext.h \
    src/IAppStateScreen.h \
    src/EditorScreen.h \
    src/Systems/ThreadedSystem.h \
    src/Systems/DatabaseManager.h \
    src/Systems/Project.h \
    src/Systems/Scene.h \
    src/Systems/Resource.h \
    src/Systems/ISerializable.h \
    src/Editor/EditorChoiceScreen.h \
    src/Editor/EditorMainScreen.h \
    src/Editor/Resources/ResourceTreeItem.h \
    src/Editor/Resources/ResourceTreeItemModel.h \
    src/Editor/EditorResourceWidget.h \
    src/Widgets/ProjectCardSelectionWidget.h \
    src/Widgets/OverlayBase.h \
    src/Editor/Resources/WebResourceOverlay.h \
    src/Widgets/Player/MediaPlayer.h   \
    src/Widgets/ResourceDisplayWidget.h \
    src/Editor/EditorActionBar.h \
    src/Editor/EditorWidgetBase.h \
    src/Editor/EditorResourceDisplayWidget.h \
    src/Editor/Resources/ResourceTreeItemSortFilterProxyModel.h \
    src/Editor/EditorSceneNodeWidget.h \
    src/Editor/NodeEditor/SceneNodeModel.h \
    src/Editor/NodeEditor/SceneTranstitionData.h \
    src/Editor/NodeEditor/SceneNodeModelWidget.h \
    src/Editor/NodeEditor/StartNodeModel.h \
    src/Editor/NodeEditor/EndNodeModel.h \
    src/Editor/NodeEditor/FlowView.h \
    src/Editor/NodeEditor/PathMergerModel.h \
    src/Editor/NodeEditor/PathSplitterModel.h \
    src/Editor/NodeEditor/PathSplitterModelWidget.h \
    src/Systems/ScriptRunner.h \
    src/Editor/EditorCodeWidget.h \
    src/Editor/Script/ScriptEditorWidget.h \
    src/Editor/Script/ScriptHighlighter.h \
    src/Systems/ScriptBackground.h \
    src/Systems/ScriptMediaPlayer.h \
    src/Systems/ScriptTextBox.h \
    src/Systems/ScriptTimer.h \
    src/Systems/ScriptThread.h \
    src/Systems/ScriptIcon.h \
    src/SceneScreen.h \
    src/Player/SceneMainScreen.h \
    src/Player/TextBoxWidget.h \
    src/Widgets/IWidgetBaseInterface.h \
    src/Player/InformationWidget.h \
    src/Player/ProjectRunner.h \
    src/Systems/ScriptRunnerSignalEmiter.h \
    src/Player/FlowLayout.h \
    src/Player/TimerDisplayWidget.h \
    src/Player/TimerWidget.h \
    src/Player/BackgroundWidget.h \
    src/Constants.h \
    src/Style.h \
    src/Widgets/MenuButton.h \
    src/Widgets/TitleLabel.h \
    src/Editor/Script/ResourceSnippetOverlay.h \
    src/Editor/Script/IconSnippetOverlay.h \
    src/Editor/Script/BackgroundSnippetOverlay.h \
    src/Editor/Script/TextSnippetOverlay.h \
    src/Editor/Script/TimerSnippetOverlay.h \
    src/Widgets/ColorPicker.h \
    version.h \
    src/CreditsScreen.h \
    src/Widgets/SearchWidget.h \
    src/Systems/ScriptStorage.h \
    src/UISoundEmitter.h \
    src/Editor/Script/ThreadSnippetOverlay.h \
    src/Editor/EditorModel.h \
    src/SVersion.h \
    src/Editor/Script/ScriptEditorModel.h \
    src/Widgets/HelpOverlay.h \
    src/Systems/HelpFactory.h \
    src/Systems/OverlayManager.h \
    src/Editor/EditorProjectSettingsWidget.h

FORMS += \
    src/MainWindow.ui \
    src/MainScreen.ui \
    src/SettingsScreen.ui \
    src/EditorScreen.ui \
    src/Editor/EditorChoiceScreen.ui \
    src/Editor/EditorMainScreen.ui \
    src/Editor/EditorResourceWidget.ui \
    src/Widgets/ProjectCardSelectionWidget.ui \
    src/Editor/Resources/WebResourceOverlay.ui \
    src/Widgets/ResourceDisplayWidget.ui \
    src/Editor/EditorActionBar.ui \
    src/Editor/EditorResourceDisplayWidget.ui \
    src/Editor/EditorSceneNodeWidget.ui \
    src/Editor/NodeEditor/SceneNodeModelWidget.ui \
    src/Editor/NodeEditor/PathSplitterModelWidget.ui \
    src/Editor/EditorCodeWidget.ui \
    src/SceneScreen.ui \
    src/Player/SceneMainScreen.ui \
    src/Player/TextBoxWidget.ui \
    src/Player/InformationWidget.ui \
    src/Player/TimerDisplayWidget.ui \
    src/Editor/Script/ResourceSnippetOverlay.ui \
    src/Editor/Script/IconSnippetOverlay.ui \
    src/Editor/Script/BackgroundSnippetOverlay.ui \
    src/Editor/Script/TextSnippetOverlay.ui \
    src/Editor/Script/TimerSnippetOverlay.ui \
    src/Widgets/ColorPicker.ui \
    src/CreditsScreen.ui \
    src/Widgets/SearchWidget.ui \
    src/Editor/Script/ThreadSnippetOverlay.ui \
    src/Widgets/HelpOverlay.ui \
    src/Editor/EditorProjectSettingsWidget.ui

RESOURCES += \
    resources.qrc

RC_ICONS = resources/img/Icon.ico
RC_FILE = version_win.rc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
