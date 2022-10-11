execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/bin")
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/data")
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/help")
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/license")
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/plugins")

# for now we don't need this stuff
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/bin/data")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/include")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/lib")
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/lib")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/styles")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/mkspecs")

find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_INSTALL_BINS}")
if (KDE_DEBUG)
  set(reldebFlagWindeploy debug)
  set(debugSuffix "d")
else()
  set(reldebFlagWindeploy release)
  set(debugSuffix "")
endif()


function(ExecuteDeployQt fileToDep dirToDep dirQml)
  execute_process(COMMAND "${WINDEPLOYQT_EXECUTABLE}" --dir "${dirToDep}"
    --verbose 0 --no-compiler-runtime --no-patchqt --angle --${reldebFlagWindeploy}
    --qmldir "${dirQml}"
    "${fileToDep}")
endfunction(ExecuteDeployQt)

execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${CMAKE_SOURCE_DIR}/bin/qt.conf"
  "${CMAKE_INSTALL_PREFIX}/bin/qt.conf")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${CMAKE_BINARY_DIR}/bin/JOIPEngine${CMAKE_EXECUTABLE_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin/JOIPEngine${CMAKE_EXECUTABLE_SUFFIX}")

ExecuteDeployQt("${CMAKE_INSTALL_PREFIX}/bin/JOIPEngine${CMAKE_EXECUTABLE_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin"
  "${CMAKE_SOURCE_DIR}/resources/qml")

# remove unused stuff
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/bin/position")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/bin/resources")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/bin/webview")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/bin/QtWebEngine")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/bin/QtWebView")

execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/bin/Qt5Positioning*")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/bin/Qt5WebEngineCore*")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/bin/Qt5WebEngine*")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/bin/Qt5WebView*")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/bin/QtWebEngineProcess*")

# move plugins
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins/audio")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins/bearer")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins/iconengines")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins/imageformats")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins/mediaservice")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins/platforms")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins/playlistformats")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins/qmltooling")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins/scenegraph")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins/styles")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/audio" "${CMAKE_INSTALL_PREFIX}/plugins/audio")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/bearer" "${CMAKE_INSTALL_PREFIX}/plugins/bearer")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/iconengines" "${CMAKE_INSTALL_PREFIX}/plugins/iconengines")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/imageformats" "${CMAKE_INSTALL_PREFIX}/plugins/imageformats")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/mediaservice" "${CMAKE_INSTALL_PREFIX}/plugins/mediaservice")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/platforms" "${CMAKE_INSTALL_PREFIX}/plugins/platforms")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/playlistformats" "${CMAKE_INSTALL_PREFIX}/plugins/playlistformats")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/qmltooling" "${CMAKE_INSTALL_PREFIX}/plugins/qmltooling")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/scenegraph" "${CMAKE_INSTALL_PREFIX}/plugins/scenegraph")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/styles" "${CMAKE_INSTALL_PREFIX}/plugins/styles")

# move translations
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/bin/translations/qtwebengine_locales")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/translations")
file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/translations" "${CMAKE_INSTALL_PREFIX}/translations")

# add libraries qt does not recognize as required
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${QT_INSTALL_BINS}/Qt5OpenGL${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin/Qt5OpenGL${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${QT_INSTALL_BINS}/Qt5MultimediaWidgets${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin/Qt5MultimediaWidgets${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${QT_INSTALL_BINS}/Qt5Xml${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin/Qt5Xml${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${QT_INSTALL_BINS}/Qt5MultimediaWidgets${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin/Qt5MultimediaWidgets${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${QT_INSTALL_BINS}/Qt5XmlPatterns${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin/Qt5XmlPatterns${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${QT_INSTALL_BINS}/QtAV${debugSuffix}1${CMAKE_SHARED_LIBRARY_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin/QtAV${debugSuffix}1${CMAKE_SHARED_LIBRARY_SUFFIX}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${QT_INSTALL_BINS}/QtAVWidgets${debugSuffix}1${CMAKE_SHARED_LIBRARY_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin/QtAVWidgets${debugSuffix}1${CMAKE_SHARED_LIBRARY_SUFFIX}")

if(32BIT)
  file(GLOB aditionalFiles
    "${CMAKE_SOURCE_DIR}/3rdparty/QtAV/ffmpeg/QtAV-depends-windows-x86+x64/bin/*${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${CMAKE_SOURCE_DIR}/3rdparty/win_openssl/*${CMAKE_SHARED_LIBRARY_SUFFIX}"
  )
else()
  file(GLOB aditionalFiles
    "${CMAKE_SOURCE_DIR}/3rdparty/QtAV/ffmpeg/QtAV-depends-windows-x86+x64/bin/x64/*${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${CMAKE_SOURCE_DIR}/3rdparty/win_openssl/*x64${CMAKE_SHARED_LIBRARY_SUFFIX}"
  )
endif()
file(COPY ${aditionalFiles} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

# We need this to see images for some reason idk ¯\_(ツ)_/¯
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${QT_INSTALL_LIBS}/Qt5Core${debugSuffix}${CMAKE_STATIC_LIBRARY_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/lib/Qt5Core${debugSuffix}${CMAKE_STATIC_LIBRARY_SUFFIX}")

# copy styles
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory
  "${CMAKE_SOURCE_DIR}/resources/desktop_styles"
  "${CMAKE_INSTALL_PREFIX}/styles")
