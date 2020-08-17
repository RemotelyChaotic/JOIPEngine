# JOIPEngine

## Building
### Prerequisites:
- Get [Qt](https://download.qt.io/) and either install a pre-built version or build all required modules from source ( Core, Multimedia, MultimediaWidgets, Network, OpenGL, Qml, Quick, QuickControls2, QuickWidgets, Svg, Widgets, WebChannel, Xml )
- [Get](https://github.com/wang-bin/QtAV) and [build](https://github.com/wang-bin/QtAV/wiki/Build-QtAV) QtAV for your target platform with FFmpeg and OpenAL support
- Get a pre-built version of [OpenSSL](https://www.openssl.org/) or build it from source.

### Build:
Building requires [CMake](https://cmake.org/) and you can build from console or from the IDE.

#### Building in QtCreator
Open src_dir/CMakeList.txt in QtCreator -> Run CMake and build

#### Building in Console
```
cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQTDIR=C:\Qt\5.14.2\msvc2017 src_dir
ninja
```
