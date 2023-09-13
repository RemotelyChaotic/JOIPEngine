cmake_minimum_required(VERSION 3.5)

project(JOIPEngineUpdater VERSION ${JOIP_UPDATER_VERSION_MAJOR}.${JOIP_UPDATER_VERSION_MINOR}.${JOIP_UPDATER_VERSION_PATCH} LANGUAGES CXX)

include(JOIPCommon)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network QuickControls2 Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network QuickControls2 Widgets REQUIRED)

if (WIN32)
  find_package(QT NAMES Qt6 Qt5 COMPONENTS WinExtras REQUIRED)
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS WinExtras REQUIRED)
endif()

set(JOIPUpdaterSources ${CMAKE_CURRENT_LIST_DIR})
set(JOIPEngineSources ${CMAKE_SOURCE_DIR}/src)

set(Sources
  ${JOIPUpdaterSources}/main.cpp
  ${JOIPUpdaterSources}/resources.qrc
  ${JOIPUpdaterSources}/Application.cpp
  ${JOIPUpdaterSources}/Application.h
  ${JOIPUpdaterSources}/SettingsData.h
  ${JOIPUpdaterSources}/Splash.cpp
  ${JOIPUpdaterSources}/Splash.h
  ${JOIPUpdaterSources}/Updater.cpp
  ${JOIPUpdaterSources}/Updater.h
  ${JOIPEngineSources}/Style.cpp
  ${JOIPEngineSources}/Style.h
  ${JOIPEngineSources}/SVersion.h
  ${JOIPEngineSources}/Widgets/TitleLabel.cpp
  ${JOIPEngineSources}/Widgets/TitleLabel.h
  ${CMAKE_SOURCE_DIR}/version.cpp
  ${CMAKE_SOURCE_DIR}/version.h)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(${PROJECT_NAME}
      MANUAL_FINALIZATION
      ${Sources}
  )
else()
  add_executable(${PROJECT_NAME} ${Sources})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE QT_FEATURE_zstd=-1)
if (WIN32)
  if (32BIT)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DOWNLOAD_POSTFIX=32_)
  endif()
endif()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::QuickControls2
    Qt${QT_VERSION_MAJOR}::Widgets
    nlohmann_json_schema_validator
  )

if (WIN32)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
      Qt${QT_VERSION_MAJOR}::WinExtras)
endif()

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${JOIPUpdaterSources}
    ${JOIPEngineSources}
  )

#-----------------------------------------------------------------------------------------
# Version and icon
#-----------------------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set(APPNAME ${PROJECT_NAME})
set(SETICONPATH ON)
set(ICONPATH "${CMAKE_SOURCE_DIR}/src/Updater/resouces/Icon.ico")

set(${PROJECT_NAME}_VERSION_MAJOR ${JOIP_UPDATER_VERSION_MAJOR})
set(${PROJECT_NAME}_VERSION_MINOR ${JOIP_UPDATER_VERSION_MINOR})
set(${PROJECT_NAME}_VERSION_PATCH ${JOIP_UPDATER_VERSION_PATCH})

set(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")
set(VERSION_DOT ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH} CACHE INTERNAL "")
set(VERSION_COMMA ${${PROJECT_NAME}_VERSION_MAJOR},${${PROJECT_NAME}_VERSION_MINOR},${${PROJECT_NAME}_VERSION_PATCH} CACHE INTERNAL "")

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${${PROJECT_NAME}_VERSION}")

set(PROJECT_VERSION ${${PROJECT_NAME}_VERSION})

get_target_property(OUTPUT_BASENAME ${PROJECT_NAME} OUTPUT_NAME)
set(OUTPUT_FILENAME "$<TARGET_FILE_NAME:${PROJECT_NAME}>")

target_sources(${PROJECT_NAME}
  PRIVATE
  "${CMAKE_SOURCE_DIR}/version.cpp"
  "${CMAKE_SOURCE_DIR}/version.h.in"
  "${CMAKE_SOURCE_DIR}/version_win.rc.in"
  "${PROJECT_BINARY_DIR}/version_win.rc"
  )

configure_file(
  "${CMAKE_SOURCE_DIR}/version_win.rc.in"
  "${PROJECT_BINARY_DIR}/version_win.rc"
)

configure_file_generate(
  "${CMAKE_SOURCE_DIR}/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/out_Version.h"
)

#-----------------------------------------------------------------------------------------
# install
#-----------------------------------------------------------------------------------------
qt_get_variables()
install(CODE "set(32BIT ${32BIT})")
install(CODE "set(KDE_DEBUG ${KDE_DEBUG})")
install(CODE "set(CMAKE_EXECUTABLE_SUFFIX ${CMAKE_EXECUTABLE_SUFFIX})")
install(CODE "set(CMAKE_SHARED_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})")
install(CODE "set(CMAKE_STATIC_LIBRARY_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})")
install(CODE "set(QT_INSTALL_BINS ${QT_INSTALL_BINS})")
install(CODE "set(QT_INSTALL_LIBS ${QT_INSTALL_LIBS})")
install(CODE "set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})")
install(SCRIPT ${CMAKE_SOURCE_DIR}/cmake/JOIPUpdaterDeploy.cmake)

if (KDE_DEBUG)
  set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
endif()
unset(CMAKE_INSTALL_UCRT_LIBRARIES)
unset(CMAKE_INSTALL_MFC_LIBRARIES)
unset(CMAKE_INSTALL_OPENMP_LIBRARIES)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS TRUE)
include(InstallRequiredSystemLibraries)

#-----------------------------------------------------------------------------------------
# Finalization under Qt6
#-----------------------------------------------------------------------------------------
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
