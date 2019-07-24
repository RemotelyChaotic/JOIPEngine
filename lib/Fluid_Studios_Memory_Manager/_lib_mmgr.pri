INCLUDEPATH += \
  $$PWD
  
SOURCES += \
  $$PWD/mmgr.cpp
  
HEADERS += \  
   $$PWD/mmgr.h \
   $$PWD/nommgr.h

win32-msvc* {
  QMAKE_CXXFLAGS += /wd4477 /wd4117 /wd4311 /wd4302 /wd4091
}
