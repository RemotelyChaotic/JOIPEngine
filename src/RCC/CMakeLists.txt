cmake_minimum_required(VERSION 3.5)

project(rcc LANGUAGES CXX)

include(JOIPCommon)

find_package(Qt5 COMPONENTS
    Core
    Xml
  REQUIRED)

set(Sources
  main.cpp
  rcc.cpp
  rcc.h
  ../Systems/PhysFs/PhysFsFileEngine.cpp
  ../Systems/PhysFs/PhysFsFileEngine.h
  ../Utils/RaiiFunctionCaller.cpp
  ../Utils/RaiiFunctionCaller.h)

add_executable(${PROJECT_NAME} ${Sources})
set_target_properties(${PROJECT_NAME} PROPERTIES
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
  LINK_FLAGS /SUBSYSTEM:CONSOLE)

# TODO: not used in default build of Qt 5.14.2
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_FEATURE_zstd=-1)

#-----------------------------------------------------------------------------------------
# Libs
#-----------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Qt5::Core
    Qt5::Xml
    physfs-static)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../Systems/PhysFs
  ${CMAKE_CURRENT_SOURCE_DIR}/../Utils
  ${Qt5Core_PRIVATE_INCLUDE_DIRS}
  ${PhysicsFS_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------------------
# Version and icon
#-----------------------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set(APPNAME ${PROJECT_NAME})
set(SETICONPATH OFF)

set(${PROJECT_NAME}_VERSION_MAJOR 5)
set(${PROJECT_NAME}_VERSION_MINOR 14)
set(${PROJECT_NAME}_VERSION_PATCH 2)

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
