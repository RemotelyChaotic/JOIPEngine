execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/updater")

find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_INSTALL_BINS}")
if (KDE_DEBUG)
  set(reldebFlagWindeploy debug)
  set(debugSuffix "d")
else()
  set(reldebFlagWindeploy release)
  set(debugSuffix "")
endif()


function(ExecuteDeployQt fileToDep dirToDep)
  execute_process(COMMAND "${WINDEPLOYQT_EXECUTABLE}" --dir "${dirToDep}"
    --verbose 0 --no-compiler-runtime --no-patchqt --angle --${reldebFlagWindeploy}
    "${fileToDep}")
endfunction(ExecuteDeployQt)


execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${CMAKE_SOURCE_DIR}/bin/qt.conf.updater"
  "${CMAKE_INSTALL_PREFIX}/updater/qt.conf")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${CMAKE_BINARY_DIR}/bin/JOIPEngineUpdater${CMAKE_EXECUTABLE_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/updater/JOIPEngineUpdater${CMAKE_EXECUTABLE_SUFFIX}")

ExecuteDeployQt("${CMAKE_INSTALL_PREFIX}/updater/JOIPEngineUpdater${CMAKE_EXECUTABLE_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/updater")

if(32BIT)
  file(GLOB aditionalFiles
    "${CMAKE_SOURCE_DIR}/3rdparty/win_openssl/*${CMAKE_SHARED_LIBRARY_SUFFIX}"
  )
else()
  file(GLOB aditionalFiles
    "${CMAKE_SOURCE_DIR}/3rdparty/win_openssl/x64/*x64${CMAKE_SHARED_LIBRARY_SUFFIX}"
  )
endif()
file(COPY ${aditionalFiles} DESTINATION "${CMAKE_INSTALL_PREFIX}/updater")
