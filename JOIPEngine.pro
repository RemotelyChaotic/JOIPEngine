#-------------------------------------------------
#
# Project created by QtCreator 2019-07-12T20:22:42
#
#-------------------------------------------------

QT       += core gui opengl script

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
    src/Backend/DatabaseAccessObject.cpp

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
    src/Backend/DatabaseAccessObject.h

FORMS += \
    src/MainWindow.ui \
    src/MainScreen.ui \
    src/SettingsScreen.ui \
    src/SceneScreen.ui \
    src/SceneSelectionScreen.ui \
    src/EditorScreen.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
