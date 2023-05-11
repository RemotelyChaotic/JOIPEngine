cmake_minimum_required(VERSION 3.1)

project(json_lua CXX)

find_package(Qt5 COMPONENTS
    Core
    Qml
  REQUIRED)

add_library(${PROJECT_NAME} STATIC
  JsonLuaWrapper.h
  json_lua_resources.qrc
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Qt5::Core
    Qt5::Qml
)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    BUILDJSONLUAWRAPPER
)
