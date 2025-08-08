cmake_minimum_required(VERSION 3.5)

project(Rcc_static LANGUAGES CXX)

include(JOIPCommon)

find_package(Qt5 COMPONENTS
    Core
    Xml
  REQUIRED)

set(Sources
  rcc.cpp
  rcc.h
  rcc_options.h
  rcc_runner.cpp
  rcc_runner.h
)

add_library(${PROJECT_NAME} STATIC ${Sources})
add_library(Qt::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
if (WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# TODO: not used in default build of Qt 5.14.2
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_FEATURE_zstd=-1)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} BASE_NAME ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")
target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")

#-----------------------------------------------------------------------------------------
# Libs
#-----------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Qt5::Core
    Qt5::Xml
  )

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
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

if (WIN32)
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
else()
  target_sources(${PROJECT_NAME}
    PRIVATE
      "${CMAKE_SOURCE_DIR}/version.cpp"
      "${CMAKE_SOURCE_DIR}/version.h.in"
    )
endif()

configure_file_generate(
  "${CMAKE_SOURCE_DIR}/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/out_Version.h"
)
