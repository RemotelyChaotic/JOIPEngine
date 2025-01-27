#!/bin/bash

export QMAKE="$1/qmake"
export APPDIR="$2"
export QML_SOURCES_PATHS="$3"

export LD_LIBRARY_PATH="$1/../lib:$2/usr/lib"
export EXTRA_QT_MODULES="multimediawidgets;quickwidgets;svg;websockets;xml;xmlpatterns"

./AppImage/linuxdeploy-plugin-qt-x86_64.AppImage --appdir $APPDIR
