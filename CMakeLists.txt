cmake_minimum_required(VERSION 3.5)

# Global Project settings
set(BUILD_TESTING OFF)  # Disable tests of all libraries
set(BUILD_EXAMPLES OFF) # Disable examples for all libraries

set(JOIP_VERSION_MAJOR 1)
set(JOIP_VERSION_MINOR 8)
set(JOIP_VERSION_PATCH 0)

set(JOIP_UPDATER_VERSION_MAJOR 1)
set(JOIP_UPDATER_VERSION_MINOR 0)
set(JOIP_UPDATER_VERSION_PATCH 0)

# KDE Options
set(ECM_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake/ECM/share/ECM/cmake)
set(KDE_SKIP_TEST_SETTINGS TRUE)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Create Root Project, for Android this is our APK-target
# This must be done before any KDE shenanigans
project(JOIPEngine VERSION ${JOIP_VERSION_MAJOR}.${JOIP_VERSION_MINOR}.${JOIP_VERSION_PATCH} LANGUAGES CXX)

# KDE imports; for android no imports, but set debug cache variable manually
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Default")
  set(KDE_DEBUG ON)
endif()

# define LINUX variable for ease of access
if(UNIX AND NOT APPLE AND NOT ANDROID)
    set(LINUX TRUE)
endif()

#include scripts
include(src/JOIPEngineProject.cmake)
include(JOIPCommon)

if (ANDROID)
  set(DISABLE_PARALLEL_CONFIGURE ON)
endif()
# For wondows & Linux use KDE common settings
if(NOT ANDROID)
  include(KDEInstallDirs)
  include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
  include(KDECMakeSettings)
  include(KDEClangFormat)
  # overwrite install dir
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/deploy" CACHE PATH "Install path prefix" FORCE)
endif()

# check 32bit
set(32BIT FALSE CACHE INTERNAL "")
if(NOT "${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    if(NOT CMAKE_CL_64)
      set(32BIT TRUE CACHE INTERNAL "")
    else()
      set(32BIT FALSE CACHE INTERNAL "")
    endif()
  else()
    set(32BIT FALSE CACHE INTERNAL "")
  endif()
else()
  set(32BIT FALSE CACHE INTERNAL "")
endif()
message("-- 32 Bit Arch: ${32BIT}")

# custom options that override KDE options and are the only relevant ones for android
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# find_package QT
FindRequiredQtPackages()

# Android setup
if (ANDROID)
  set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

  set(ANDROID_EXTRA_PLUGINS
    "${CMAKE_BINARY_DIR}/android-build/libs/${ANDROID_ABI}/plugins"
  CACHE INTERNAL "")

  set(ANDROID_MIN_SDK_VERSION 21)
  set(ANDROID_PACKAGE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/android-sources CACHE INTERNAL "")
endif()

# actual targets
if(WIN32 OR LINUX)
  #libs
  add_subdirectory(3rdparty)
  include(QtAV/FindQtAV)

  # executable targets
  add_subdirectory(src)
  # Only for MSVC builds
  if(MSVC)
    add_subdirectory(src/Windows/Launcher)
  endif()
  # Only supported on Windows so far
  if (WIN32)
    add_subdirectory(src/Updater)
  endif()

elseif (ANDROID)
  # For android the executable project *needs* to be in the root project folder because of
  # bugs with qt
  CreateJOIPProject(JOIPEngine)

  # DUMB AS FRICK BUGFIX THAT *SHOULD* BE FIXED WITH NEWER QTCREATOR
  # see https://bugreports.qt.io/browse/QTCREATORBUG-22742
  list(REMOVE_DUPLICATES CUSTOM_ANDROID_EXTRA_LIBS)
  string(REPLACE ";" "," CUSTOM_ANDROID_EXTRA_LIBS_STRING "${CUSTOM_ANDROID_EXTRA_LIBS}")
  set(ANDROID_EXTRA_LIBS "${CUSTOM_ANDROID_EXTRA_LIBS_STRING}" CACHE INTERNAL "")

  # libs
  add_subdirectory(3rdparty)
  include(QtAV/FindQtAV)

  # apk setup
  JOIPProjectSettings(JOIPEngine)
  FinalizeJOIPProject(JOIPEngine)
endif()
