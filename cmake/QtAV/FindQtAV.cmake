# - Try to find the QtAV library
#
# Once done this will define
#
#  QTAV_FOUND        - system has libqtav
#  QTAV_INCLUDE_DIRS - the libqtav include directory
#  QTAV_LIBRARIES    - Link these to use libqtav

find_package(Qt5 QUIET REQUIRED NO_MODULE COMPONENTS Core)

get_target_property(qmake Qt5::qmake LOCATION)
execute_process(
    COMMAND ${qmake} -query QT_INSTALL_HEADERS
    OUTPUT_VARIABLE QT_INSTALL_HEADERS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${qmake} -query QT_INSTALL_LIBS
    OUTPUT_VARIABLE QT_INSTALL_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

find_path(QTAV_INCLUDE_DIR NAMES QtAV.h
    HINTS ${QT_INSTALL_HEADERS}
    PATH_SUFFIXES QtAV
)

if(ANDROID)
  if (KDE_DEBUG)
    find_library(QTAV_LIBRARY NAMES
        libQtAVd${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
        libQmlAVd${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
        HINTS ${QT_INSTALL_LIBS})
  else()
    find_library(QTAV_LIBRARY NAMES
        libQtAV${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
        libQmlAV${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
        HINTS ${QT_INSTALL_LIBS})
  endif(KDE_DEBUG)
else()
  if (KDE_DEBUG)
    find_library(QTAV_LIBRARY NAMES QtAVd QtAVd1
        HINTS ${QT_INSTALL_LIBS})
  else()
    find_library(QTAV_LIBRARY NAMES QtAV QtAV1
        HINTS ${QT_INSTALL_LIBS})
  endif(KDE_DEBUG)
endif()

find_path(QTAVWIDGETS_INCLUDE_DIR NAMES QtAVWidgets.h
    HINTS ${QT_INSTALL_HEADERS}
    PATH_SUFFIXES QtAVWidgets
)
if(ANDROID)
  if (KDE_DEBUG)
    find_library(QTAVWIDGETS_LIBRARY NAMES libQtAVWidgetsd${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
        HINTS ${QT_INSTALL_LIBS}
    )
  else()
    find_library(QTAVWIDGETS_LIBRARY NAMES libQtAVWidgets${CMAKE_SHARED_LIBRARY_SUFFIX_CXX}
        HINTS ${QT_INSTALL_LIBS}
    )
  endif(KDE_DEBUG)
else()
  if (KDE_DEBUG)
    find_library(QTAVWIDGETS_LIBRARY NAMES QtAVWidgetsd QtAVWidgetsd1
        HINTS ${QT_INSTALL_LIBS}
    )
  else()
    find_library(QTAVWIDGETS_LIBRARY NAMES QtAVWidgets QtAVWidgets1
        HINTS ${QT_INSTALL_LIBS}
    )
  endif(KDE_DEBUG)
endif()

set(QTAV_INCLUDE_DIRS ${QTAV_INCLUDE_DIR} ${QTAV_INCLUDE_DIR}/..)
set(QTAV_LIBRARIES ${QTAV_LIBRARY})
if(NOT QTAVWIDGETS_INCLUDE_DIR MATCHES "QTAVWIDGETS_INCLUDE_DIR-NOTFOUND")
    set(QTAVWIDGETS_INCLUDE_DIRS ${QTAVWIDGETS_INCLUDE_DIR} ${QTAVWIDGETS_INCLUDE_DIR}/.. ${QTAV_INCLUDE_DIRS})
endif()
if(NOT QTAV_LIBRARIES MATCHES "QTAV_LIBRARIES-NOTFOUND")
    set(QTAVWIDGETS_LIBRARIES ${QTAVWIDGETS_LIBRARY} ${QTAV_LIBRARY})
endif()

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(QtAV REQUIRED_VARS QTAV_LIBRARIES QTAV_INCLUDE_DIRS)
mark_as_advanced(QTAV_INCLUDE_DIRS QTAV_LIBRARIES QTAVWIDGETS_INCLUDE_DIRS QTAVWIDGETS_LIBRARIES)

# add as library
add_library(Qt::AV UNKNOWN IMPORTED)
set_target_properties(Qt::AV PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${QTAV_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${QTAV_INCLUDE_DIRS}"
  IMPORTED_GLOBAL TRUE
)
add_library(Qt::AVWidgets UNKNOWN IMPORTED)
set_target_properties(Qt::AVWidgets PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${QTAVWIDGETS_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${QTAVWIDGETS_INCLUDE_DIRS}"
  IMPORTED_GLOBAL TRUE
)

message("QtAV_FOUND                 = ${QTAV_FOUND}")
message("QTAV_INCLUDE_DIRS          = ${QTAV_INCLUDE_DIRS}")
message("QTAV_LIBRARIES             = ${QTAV_LIBRARIES}")
message("QTAVWIDGETS_INCLUDE_DIRS   = ${QTAVWIDGETS_INCLUDE_DIRS}")
message("QTAVWIDGETS_LIBRARIES      = ${QTAVWIDGETS_LIBRARIES}")

if (ANDROID)
  if (NOT (QT_VERSION LESS 5.14.0))
    set(ffmpeg_dir ${CMAKE_SOURCE_DIR}/3rdparty/QtAV/ffmpeg/ffmpeg-4.2-android-clang/lib/${ANDROID_ABI})
  else()
    set(ffmpeg_dir ${CMAKE_SOURCE_DIR}/3rdparty/QtAV/ffmpeg/ffmpeg-4.2-android-clang/lib/${ANDROID_TARGET_ARCH})
  endif()

  set(FFMPEG_INCLUDE_DIRS ${ffmpeg_dir}/../../include)

  message("QTAV Android               = ${ffmpeg_dir}")
  set(CUSTOM_ANDROID_EXTRA_LIBS
      ${CUSTOM_ANDROID_EXTRA_LIBS}
      ${ffmpeg_dir}/libavcodec.so
      ${ffmpeg_dir}/libavdevice.so
      ${ffmpeg_dir}/libavfilter.so
      ${ffmpeg_dir}/libavformat.so
      ${ffmpeg_dir}/libavutil.so
      ${ffmpeg_dir}/libavresample.so
      ${ffmpeg_dir}/libffmpeg.so
      ${ffmpeg_dir}/libswresample.so
      ${ffmpeg_dir}/libswscale.so
  CACHE INTERNAL "")

elseif(WIN32)
  if (32BIT)
    set(ffmpeg_dir ${CMAKE_SOURCE_DIR}/3rdparty/QtAV/ffmpeg/QtAV-depends-windows-x86+x64/bin)
    set(FFMPEG_INCLUDE_DIRS ${ffmpeg_dir}/../include)
  else()
    set(ffmpeg_dir ${CMAKE_SOURCE_DIR}/3rdparty/QtAV/ffmpeg/QtAV-depends-windows-x86+x64/bin/x64)
    set(FFMPEG_INCLUDE_DIRS ${ffmpeg_dir}/../../include)
  endif()

  message("QTAV Windows               = ${ffmpeg_dir}")
  set(ffmpeg_libs)
  file(GLOB ffmpeg_libs ${ffmpeg_dir}/*.dll)
  set(CUSTOM_DESKTOP_EXTRA_LIBS ${ffmpeg_libs} CACHE INTERNAL "")
elseif(LINUX)
  if (32BIT)
    message(WARNING "Linux 32bit ffmpeg unsupported!")
    set(ffmpeg_dir)
    set(FFMPEG_INCLUDE_DIRS)
  else()
    set(ffmpeg_dir ${CMAKE_SOURCE_DIR}/3rdparty/QtAV/ffmpeg/ffmpeg-4.3-linux-gcc/lib)
    set(FFMPEG_INCLUDE_DIRS ${ffmpeg_dir}/../include)
  endif()

  target_link_directories(Qt::AV INTERFACE ${ffmpeg_dir})
  target_link_directories(Qt::AVWidgets INTERFACE ${ffmpeg_dir})

  message("QTAV Linux                 = ${ffmpeg_dir}")
  set(ffmpeg_libs)
  file(GLOB ffmpeg_libs ${ffmpeg_dir}/*.dll)
  set(CUSTOM_DESKTOP_EXTRA_LIBS ${ffmpeg_libs} CACHE INTERNAL "")
endif()

message("FFMPEG_INCLUDE_DIRS        = ${FFMPEG_INCLUDE_DIRS}")
