#-------------------------------------------------
#
# Project created by QtCreator 2019-07-12T20:22:42
#
#-------------------------------------------------

QT       += core gui multimedia opengl script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    src/Scenes/ProjectCardWidget.cpp \
    src/Scenes/ProjectCardSelectionWidget.cpp

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
    src/Scenes/ProjectCardWidget.h \
    src/Scenes/ProjectCardSelectionWidget.h

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
    src/Scenes/ProjectCardWidget.ui \
    src/Scenes/ProjectCardSelectionWidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
