cmake_minimum_required(VERSION 3.5)

# disable testing and examples
set(BUILD_TESTING OFF)
set(BUILD_EXAMPLES OFF)

# KDE Options
set(ECM_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake/ECM/share/ECM/cmake)
set(KDE_SKIP_TEST_SETTINGS TRUE)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# hack for android: root Project has to have the same name as the apk Target
set(JOIP_PROJECT_NAME JOIPEngine)
project(${JOIP_PROJECT_NAME})

include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEClangFormat)

# custom options
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Android setup
if (ANDROID)
  set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

  set(QT_ANDROID_EXTRA_PLUGINS
    "${CMAKE_BINARY_DIR}/android-build/libs/${ANDROID_ABI}/plugins"
  CACHE INTERNAL "")

  set(ANDROID_MIN_SDK_VERSION 21)
  set(ANDROID_PACKAGE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/android-sources CACHE INTERNAL "")
endif()

# projects
add_subdirectory(3rdparty)
add_subdirectory(src)
if(WIN32 AND MSVC)
  add_subdirectory(src/Launcher)
  add_subdirectory(src/RCC)
endif()

if (ANDROID)
  # DUMB AS FUCK BUGFIX THAT WAS SUPPOSED TO BE FIXED
  # see https://bugreports.qt.io/browse/QTCREATORBUG-22742
  list(REMOVE_DUPLICATES CUSTOM_ANDROID_EXTRA_LIBS)
  string(REPLACE ";" "," CUSTOM_ANDROID_EXTRA_LIBS_STRING "${CUSTOM_ANDROID_EXTRA_LIBS}")
  set(ANDROID_EXTRA_LIBS "${CUSTOM_ANDROID_EXTRA_LIBS_STRING}" CACHE INTERNAL "")
endif()
