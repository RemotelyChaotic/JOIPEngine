cmake_minimum_required(VERSION 3.1)

project(lua_sandbox CXX)

find_package(Qt5 COMPONENTS
    Core
    Qml
  REQUIRED)

add_library(${PROJECT_NAME} STATIC
  LuaSandboxWrapper.h
  lua_sandbox_resources.qrc
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Qt5::Core
    Qt5::Qml
)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    BUILDLUASANDBOX
)
