# JOIPEngine

## Building
### Prerequisites:
- Building requires [CMake](https://cmake.org/).
- Building QtAV with CMake also requires [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/) for easy installation, see [how-to-install-pkg-config-in-windows](https://stackoverflow.com/questions/1710922/how-to-install-pkg-config-in-windows).
- A version of Pearl is also required for building the highlight definitions. Recommended for Windows is: [Strawberry Pearl](https://strawberryperl.com/)
- Get [Qt 5.14+.*](https://download.qt.io/) and either install a pre-built version or build all required modules from source ( Core Multimedia MultimediaWidgets Network PrintSupport Qml Quick QuickControls2 QuickWidgets Svg Widgets WebChannel Xml XmlPatterns). Qt 6 is not fully supported as some libraries do not support it and a lot of the API has changed.
- Build the provided [QtAV](https://github.com/RemotelyChaotic/QtAV) fork for your target platform with FFmpeg and OpenAL support. A CMake build is recommended. If you want to build it with qmake and have trubble building for Android, follow this [guide](https://github.com/wang-bin/QtAV/issues/1262#issuecomment-597193360) and build all architectures separately. Don't forget, that mingw32-make.exe must be in the Path for qmake builds.
- Get a pre-built version of [OpenSSL 1.1.*](https://www.openssl.org/), build it from source, or use the provided version in 3rd-party (1.1.1g was used for testing but newer versions should work).

### Build:
For Android, add jom.exe to your PATH, be it in the Qt-Creator Project settings, or in the console.
It's typically in a path like [...]\Qt5.14.1\Tools\QtCreator\bin\ if you downloaded Qt.
On startup, if you get an error that libavutil.so or similar was not found, check if <build>/android-build/libs/<abi>/ contains the av libraries and if not, rerun cmake.

Next you can build the Application.

#### Building in QtCreator
Open src_dir/CMakeList.txt in QtCreator -> Run CMake and build

#### Building and installing in Console with Ninja
```
cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQTDIR=C:\Qt\5.14.2\msvc2017 src_dir
cmake --build . --target install
```

#### Deploying
Install creates a directory in your build directory called deploy.

Now you can:
- copy all license files from the sources to deploy/license

And you are done.
