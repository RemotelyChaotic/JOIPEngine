# JOIPEngine

## Building
### Prerequisites:
- Building requires [CMake](https://cmake.org/).
- Get [Qt 5.14.*](https://download.qt.io/) and either install a pre-built version or build all required modules from source ( Core, Multimedia, MultimediaWidgets, Network, OpenGL, Qml, Quick, QuickControls2, QuickWidgets, Svg, Widgets, WebChannel, Xml )
- [Build](https://github.com/wang-bin/QtAV/wiki/Build-QtAV) the provided QtAV fork for your target platform with FFmpeg and OpenAL support.
- Get a pre-built version of [OpenSSL 1.1.*](https://www.openssl.org/) or build it from source (1.1.1g used for testing but others may work).

### Build:
First you will need to "install" ECM. For that you need to do the following:
```
cd /d <root>/cmake
mkdir ECM
cd ECM
cmake -G"Ninja" -DCMAKE_INSTALL_PREFIX="./" ../extra-cmake-modules
ninja install
```

Next you can build the Application.

#### Building in QtCreator
Open src_dir/CMakeList.txt in QtCreator -> Run CMake and build

#### Building in Console with Ninja
```
cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQTDIR=C:\Qt\5.14.2\msvc2017 src_dir
ninja
```

#### Deploying
First you need to run the following command (assuming Qt is installed in the default install directory)
```
> C:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin\windeployqt.exe --no-patchqt --release --qmldir "%SOURCE_DIR%\resources\qml" --no-webenginecore --no-webengine --no-webenginewidgets --no-webview --angle JOIPEngine.exe
```

Next:
- copy built OpenSSL binaries to bin
- copy QtAV binaries including all ffmpeg libraries to bin
- copy Qt5OpenGL.dll to bin
- copy qpcxd.dll from lib subfolder in the build tree to bin/imageformats
- remove the folders resources, position, QtWebEngine, QtWebView
- Then move the transplations folder up once
- copy qt.conf from <SOURCE_DIR> to bin
- copy all license files from the sources to license
- copy compiler libraries to bin (e.g vcruntime140.dll)