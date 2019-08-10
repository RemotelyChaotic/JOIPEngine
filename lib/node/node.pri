CONFIG(debug, debug|release) {
   NODE_PATH = $$PWD/debug
}
CONFIG(release, debug|release) {
   NODE_PATH = $$PWD/release
}

DEFINES += NODE_EDITOR_SHARED

INCLUDEPATH += \
    $${NODE_PATH}/include

LIBS += \
  -L$${NODE_PATH}/lib -lnodes
