cmake_minimum_required(VERSION 3.5)

project(JOIPEngineApp VERSION ${JOIP_VERSION_MAJOR}.${JOIP_VERSION_MINOR}.${JOIP_VERSION_PATCH} LANGUAGES CXX)
include(JOIPEngineProject.cmake)

CreateJOIPProject(${PROJECT_NAME})
JOIPProjectSettings(${PROJECT_NAME})

#-----------------------------------------------------------------------------------------
# install
#-----------------------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
   OUTPUT_NAME JOIPEngine)

qt_get_variables()
install(CODE "set(32BIT ${32BIT})")
install(CODE "set(KDE_DEBUG ${KDE_DEBUG})")
install(CODE "set(CMAKE_EXECUTABLE_SUFFIX ${CMAKE_EXECUTABLE_SUFFIX})")
install(CODE "set(CMAKE_SHARED_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})")
install(CODE "set(CMAKE_STATIC_LIBRARY_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})")
install(CODE "set(QT_INSTALL_BINS ${QT_INSTALL_BINS})")
install(CODE "set(QT_INSTALL_LIBS ${QT_INSTALL_LIBS})")
install(CODE "set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})")
install(SCRIPT ${CMAKE_SOURCE_DIR}/cmake/JOIPDeploy.cmake)

if (KDE_DEBUG)
  set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
endif()
unset(CMAKE_INSTALL_UCRT_LIBRARIES)
unset(CMAKE_INSTALL_MFC_LIBRARIES)
unset(CMAKE_INSTALL_OPENMP_LIBRARIES)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS TRUE)
include(InstallRequiredSystemLibraries)

#-----------------------------------------------------------------------------------------
# Finalize
#-----------------------------------------------------------------------------------------
FinalizeJOIPProject(${PROJECT_NAME})
