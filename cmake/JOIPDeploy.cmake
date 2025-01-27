if (LINUX)
  #first things first, we need to wget appimage and appimageqt
  # https://docs.appimage.org/packaging-guide/from-source/native-binaries.html#id3
  # https://github.com/linuxdeploy/linuxdeploy-plugin-qt
  execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/AppDir")
  if (NOT EXISTS "${CMAKE_INSTALL_PREFIX}/AppImage")
      execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/AppImage")
      execute_process(COMMAND "wget" "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
                      WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/AppImage")
      execute_process(COMMAND "wget" "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
                      WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/AppImage")
      execute_process(COMMAND "chmod" "+x" "linuxdeploy-x86_64.AppImage"
                      WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/AppImage")
      execute_process(COMMAND "chmod" "+x" "linuxdeploy-plugin-qt-x86_64.AppImage"
                      WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/AppImage")
  endif()
endif()

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
execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/themes")
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
  if (WIN32)
    execute_process(COMMAND "${WINDEPLOYQT_EXECUTABLE}" --dir "${dirToDep}"
      --verbose 0 --no-compiler-runtime --no-patchqt --angle --${reldebFlagWindeploy}
      --qmldir "${dirQml}"
      "${fileToDep}")
  elseif (LINUX)
    execute_process(COMMAND "${fileToDep}" "${QT_INSTALL_BINS}" "${dirToDep}" "${dirQml}"
                    WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
  endif()
endfunction(ExecuteDeployQt)

if (WIN32)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
    "${CMAKE_SOURCE_DIR}/bin/qt.conf"
    "${CMAKE_INSTALL_PREFIX}/bin/qt.conf")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
  "${CMAKE_BINARY_DIR}/bin/JOIPEngine${CMAKE_EXECUTABLE_SUFFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin/JOIPEngine${CMAKE_EXECUTABLE_SUFFIX}")

if (WIN32)
  # Do deployment now
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
  file(GLOB quick "${QT_INSTALL_LIBS}/Qt5Quick*")
  file(COPY ${quick} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
    "${QT_INSTALL_BINS}/Qt5OpenGL${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${CMAKE_INSTALL_PREFIX}/bin/Qt5OpenGL${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
    "${QT_INSTALL_BINS}/Qt5MultimediaWidgets${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${CMAKE_INSTALL_PREFIX}/bin/Qt5MultimediaWidgets${debugSuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  file(GLOB xml "${QT_INSTALL_BINS}/Qt5Xml*")
  file(COPY ${xml} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
    "${QT_INSTALL_BINS}/QtAV${debugSuffix}1${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${CMAKE_INSTALL_PREFIX}/bin/QtAV${debugSuffix}1${CMAKE_SHARED_LIBRARY_SUFFIX}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
    "${QT_INSTALL_BINS}/QtAVWidgets${debugSuffix}1${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${CMAKE_INSTALL_PREFIX}/bin/QtAVWidgets${debugSuffix}1${CMAKE_SHARED_LIBRARY_SUFFIX}")
elseif(LINUX)
  #remove old stuff
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/AppDir/usr")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/AppDir/apprun-hooks")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/lib")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/plugins")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/share")

  #remove unneeded stuff
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/bin/qtlua")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/bin/qtlua_uic")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/bin/kate-syntax-highlighter")

  execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/AppDir/usr")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/AppDir/usr/bin")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_INSTALL_PREFIX}/AppDir/usr/plugins")

  file(RENAME "${CMAKE_INSTALL_PREFIX}/bin/imageformats" "${CMAKE_INSTALL_PREFIX}/AppDir/usr/plugins/imageformats")
  file(RENAME "${CMAKE_INSTALL_PREFIX}/bin" "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib")
endif()

# ffmpeg and openssl
if (WIN32)
  if(32BIT)
    file(GLOB aditionalFiles
      "${CMAKE_SOURCE_DIR}/3rdparty/QtAV/ffmpeg/QtAV-depends-windows-x86+x64/bin/*${CMAKE_SHARED_LIBRARY_SUFFIX}"
      "${CMAKE_SOURCE_DIR}/3rdparty/win_openssl/*${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
  else()
    file(GLOB aditionalFiles
      "${CMAKE_SOURCE_DIR}/3rdparty/QtAV/ffmpeg/QtAV-depends-windows-x86+x64/bin/x64/*${CMAKE_SHARED_LIBRARY_SUFFIX}"
      "${CMAKE_SOURCE_DIR}/3rdparty/win_openssl/x64/*x64${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
  endif()
elseif(LINUX)
  file(GLOB aditionalFiles
    "${CMAKE_SOURCE_DIR}/3rdparty/QtAV/ffmpeg/ffmpeg-4.3-linux-gcc/lib/*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
    #"${CMAKE_SOURCE_DIR}/3rdparty/win_openssl/x64/*x64${CMAKE_SHARED_LIBRARY_SUFFIX}"
  )
endif()
if (WIN32)
  file(COPY ${aditionalFiles} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
elseif(LINUX)
  file(COPY ${aditionalFiles} DESTINATION "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib")
endif()

# We need this to see images for some reason idk ¯\_(ツ)_/¯
if (WIN32)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy
    "${QT_INSTALL_LIBS}/Qt5Core${debugSuffix}${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CMAKE_INSTALL_PREFIX}/lib/Qt5Core${debugSuffix}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

if (LINUX)
  # Do qt deployment now on linux
  ExecuteDeployQt("${CMAKE_SOURCE_DIR}/cmake/LinuxDeployQt.sh"
    "${CMAKE_INSTALL_PREFIX}/AppDir"
    "${CMAKE_SOURCE_DIR}/resources/qml")
endif()

# copy styles
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory
  "${CMAKE_SOURCE_DIR}/resources/desktop_styles"
  "${CMAKE_INSTALL_PREFIX}/styles")

# copy themes
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory
  "${CMAKE_SOURCE_DIR}/resources/themes"
  "${CMAKE_INSTALL_PREFIX}/themes")

# do aditional filesystem ops and appimage bundling
if (LINUX)
  # remove unused stuff
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/AppDir/usr/translations")

  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib/libQt5Positioning*")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib/libQt5WebEngineCore*")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib/libQt5WebEngine*")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib/libQt5WebView*")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -f -- "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib/libQtWebEngineProcess*")

  # libs missed by linuxdeployqt for some reason
  file(GLOB avWidgets "${QT_INSTALL_BINS}/../lib/libQtAVWidgets.*")
  file(COPY ${avWidgets} DESTINATION "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib")
  file(GLOB quickLibs "${QT_INSTALL_BINS}/../lib/libQt5Quick*so.*")
  file(COPY ${quickLibs} DESTINATION "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib")
  file(GLOB xmlLibs "${QT_INSTALL_BINS}/../lib/libQt5Xml*so.*")
  file(COPY ${xmlLibs} DESTINATION "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib")
  file(GLOB opengl "${QT_INSTALL_BINS}/../lib/libQt5OpenGL.*")
  file(COPY ${opengl} DESTINATION "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib")
  file(GLOB multimediaW "${QT_INSTALL_BINS}/../lib/libQt5MultimediaWidgets.*")
  file(COPY ${multimediaW} DESTINATION "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib")
  file(GLOB websockets "${QT_INSTALL_BINS}/../lib/libQt5WebSockets.*")
  file(COPY ${websockets} DESTINATION "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib")
  file(GLOB test "${QT_INSTALL_BINS}/../lib/libQt5Test.*")
  file(COPY ${test} DESTINATION "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib")

  # remove unneeded stuff
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/AppDir/usr/qml/QtWebEngine")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -Rf -- "${CMAKE_INSTALL_PREFIX}/AppDir/usr/qml/QtWebView")
  # copy executables and launcher script into bin again
  file(RENAME "${CMAKE_INSTALL_PREFIX}/AppDir/usr/lib/JOIPEngine" "${CMAKE_INSTALL_PREFIX}/AppDir/usr/bin/JOIPEngine")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/bin/AppRun.sh" "${CMAKE_INSTALL_PREFIX}/AppDir/AppRun")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/bin/JOIPEngine.desktop" "${CMAKE_INSTALL_PREFIX}/AppDir/JOIPEngine.desktop")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/resources/img/Icon.png" "${CMAKE_INSTALL_PREFIX}/AppDir/JOIPEngine.png")

  # ./AppImage/linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage
  execute_process(COMMAND "${CMAKE_SOURCE_DIR}/cmake/LinuxDeploy.sh" "${QT_INSTALL_BINS}" "${CMAKE_INSTALL_PREFIX}/AppDir" "${CMAKE_SOURCE_DIR}/resources/qml"
                  WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
endif()
