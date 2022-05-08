cmake_minimum_required(VERSION 3.5)

project(JOIPEngineApp VERSION ${JOIP_VERSION_MAJOR}.${JOIP_VERSION_MINOR}.${JOIP_VERSION_PATCH} LANGUAGES CXX)
include(JOIPEngineProject.cmake)

CreateJOIPProject(${PROJECT_NAME})
JOIPProjectSettings(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
   OUTPUT_NAME JOIPEngine)

FinalizeJOIPProject(${PROJECT_NAME})
