cmake_minimum_required(VERSION 3.5)

project(JOIPEngineProject)

# disable testing and examples
set(BUILD_TESTING OFF)
set(BUILD_EXAMPLES OFF)

# KDE Options
set(ECM_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake/ECM/share/ECM/cmake)
set(KDE_SKIP_TEST_SETTINGS TRUE)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
if(POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEClangFormat)

# custom options
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
#set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

if(ANDROID)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    if (ANDROID_ABI STREQUAL "armeabi-v7a")
        set(ANDROID_EXTRA_LIBS
            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
    endif()
endif()

add_subdirectory(3rdparty)
add_subdirectory(src)
if(WIN32 AND MSVC)
add_subdirectory(src/Launcher)
endif()
