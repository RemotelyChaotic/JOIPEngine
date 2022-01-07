cmake_minimum_required(VERSION 3.5)

project(JOIPEngineLauncher LANGUAGES CXX)

include(JOIPCommon)

set(Sources
  main.cpp
  ../../version.cpp
  ../../version.h)

add_executable(${PROJECT_NAME} ${Sources})
set_target_properties(${PROJECT_NAME} PROPERTIES
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
  LINK_FLAGS /SUBSYSTEM:CONSOLE)

#-----------------------------------------------------------------------------------------
# Version and icon
#-----------------------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set(APPNAME ${PROJECT_NAME})
set(SETICONPATH ON)
set(ICONPATH "${CMAKE_SOURCE_DIR}/resources/img/Icon.ico")

set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MINOR 2)
set(${PROJECT_NAME}_VERSION_PATCH 1)

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
