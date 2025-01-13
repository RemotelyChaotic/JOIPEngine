cmake_minimum_required(VERSION 3.5)

project(JOIPEngineLauncher VERSION ${JOIP_VERSION_MAJOR}.${JOIP_VERSION_MINOR}.${JOIP_VERSION_PATCH} LANGUAGES CXX)

include(JOIPCommon)

set(Sources
  main.cpp
  ${CMAKE_SOURCE_DIR}/version.cpp
  ${CMAKE_SOURCE_DIR}/version.h)

add_executable(${PROJECT_NAME} ${Sources})
if (WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    LINK_FLAGS /SUBSYSTEM:CONSOLE)
endif()

#-----------------------------------------------------------------------------------------
# Version and icon
#-----------------------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set(APPNAME ${PROJECT_NAME})
set(SETICONPATH ON)
set(ICONPATH "${CMAKE_SOURCE_DIR}/resources/img/Icon.ico")

set(${PROJECT_NAME}_VERSION_MAJOR ${JOIP_VERSION_MAJOR})
set(${PROJECT_NAME}_VERSION_MINOR ${JOIP_VERSION_MINOR})
set(${PROJECT_NAME}_VERSION_PATCH ${JOIP_VERSION_PATCH})

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
install(
  TARGETS ${PROJECT_NAME}
  DESTINATION "${CMAKE_INSTALL_PREFIX}")

if (KDE_DEBUG)
  set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
endif()
unset(CMAKE_INSTALL_UCRT_LIBRARIES)
unset(CMAKE_INSTALL_MFC_LIBRARIES)
unset(CMAKE_INSTALL_OPENMP_LIBRARIES)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS TRUE)
include(InstallRequiredSystemLibraries)
