#-------------------------------------------------
#
# Project created by QtCreator 2019-07-12T20:22:42
#
#-------------------------------------------------

QT       += \
  core \
  gui \
  multimedia \
  multimediawidgets \
  network \
  opengl \
  script \
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
CONFIG(debug, debug|release) {
   include(lib/Fluid_Studios_Memory_Manager/_lib_mmgr.pri)
}
include (lib/modeltest/modeltest.pri)
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
    src/SceneScreen.cpp \
    src/SceneSelectionScreen.cpp \
    src/WindowContext.cpp \
    src/EditorScreen.cpp \
    src/Backend/ThreadedSystem.cpp \
    src/Backend/DatabaseManager.cpp \
    src/Backend/Project.cpp \
    src/Backend/Scene.cpp \
    src/Backend/Resource.cpp \
    src/Editor/EditorChoiceScreen.cpp \
    src/Editor/EditorMainScreen.cpp \
    src/Editor/ResourceTreeItem.cpp \
    src/Editor/ResourceTreeItemModel.cpp \
    src/Editor/EditorResourceWidget.cpp \
    src/Widgets/ProjectCardSelectionWidget.cpp \
    src/Widgets/OverlayBase.cpp \
    src/Editor/WebResourceOverlay.cpp \
    src/Widgets/Player/MediaPlayer.cpp \
    src/Widgets/ResourceDisplayWidget.cpp \
    src/Editor/EditorActionBar.cpp \
    src/Editor/EditorWidgetBase.cpp \
    src/Editor/EditorResourceDisplayWidget.cpp \
    src/Editor/ResourceTreeItemSortFilterProxyModel.cpp \
    src/Editor/EditorSceneNodeWidget.cpp \
    src/Editor/NodeEditor/SceneNodeModel.cpp \
    src/Editor/NodeEditor/SceneTranstitionData.cpp \
    src/Editor/NodeEditor/SceneNodeModelWidget.cpp \
    src/Editor/NodeEditor/StartNodeModel.cpp \
    src/Editor/NodeEditor/EndNodeModel.cpp \
    src/Editor/NodeEditor/FlowView.cpp \
    src/Editor/NodeEditor/PathMergerModel.cpp \
    src/Editor/NodeEditor/PathSplitterModel.cpp \
    src/Editor/NodeEditor/PathSplitterModelWidget.cpp

HEADERS += \
    src/MainWindow.h \
    src/Settings.h \
    src/Application.h \
    src/MainScreen.h \
    src/SettingsScreen.h \
    src/SceneScreen.h \
    src/SceneSelectionScreen.h \
    src/Enums.h \
    src/WindowContext.h \
    src/IAppStateScreen.h \
    src/EditorScreen.h \
    src/Backend/ThreadedSystem.h \
    src/Backend/DatabaseManager.h \
    src/Backend/Project.h \
    src/Backend/Scene.h \
    src/Backend/Resource.h \
    src/Backend/ISerializable.h \
    src/Editor/EditorChoiceScreen.h \
    src/Editor/EditorMainScreen.h \
    src/Editor/ResourceTreeItem.h \
    src/Editor/ResourceTreeItemModel.h \
    src/Editor/EditorResourceWidget.h \
    src/Widgets/ProjectCardSelectionWidget.h \
    src/Widgets/OverlayBase.h \
    src/Editor/WebResourceOverlay.h \
    src/Widgets/Player/MediaPlayer.h   \
    src/Widgets/ResourceDisplayWidget.h \
    src/Editor/EditorActionBar.h \
    src/Editor/EditorWidgetBase.h \
    src/Editor/EditorResourceDisplayWidget.h \
    src/Editor/ResourceTreeItemSortFilterProxyModel.h \
    src/Editor/EditorSceneNodeWidget.h \
    src/Editor/NodeEditor/SceneNodeModel.h \
    src/Editor/NodeEditor/SceneTranstitionData.h \
    src/Editor/NodeEditor/SceneNodeModelWidget.h \
    src/Editor/NodeEditor/StartNodeModel.h \
    src/Editor/NodeEditor/EndNodeModel.h \
    src/Editor/NodeEditor/FlowView.h \
    src/Editor/NodeEditor/PathMergerModel.h \
    src/Editor/NodeEditor/PathSplitterModel.h \
    src/Editor/NodeEditor/PathSplitterModelWidget.h

FORMS += \
    src/MainWindow.ui \
    src/MainScreen.ui \
    src/SettingsScreen.ui \
    src/SceneScreen.ui \
    src/SceneSelectionScreen.ui \
    src/EditorScreen.ui \
    src/Editor/EditorChoiceScreen.ui \
    src/Editor/EditorMainScreen.ui \
    src/Editor/EditorResourceWidget.ui \
    src/Widgets/ProjectCardSelectionWidget.ui \
    src/Editor/WebResourceOverlay.ui \
    src/Widgets/ResourceDisplayWidget.ui \
    src/Editor/EditorActionBar.ui \
    src/Editor/EditorResourceDisplayWidget.ui \
    src/Editor/EditorSceneNodeWidget.ui \
    src/Editor/NodeEditor/SceneNodeModelWidget.ui \
    src/Editor/NodeEditor/PathSplitterModelWidget.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
