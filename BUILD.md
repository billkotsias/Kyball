# Building Kyball

Linux and Android builds use the CMake targets in the repository root. Ogre **1.10.12** (not Ogre-Next) is fetched and built with the legacy OpenGL render system on Linux and GLES2 on Android. Materials could not be inspected: `Kyball_installer.exe` was not on this machine, so the fixed-function path is the default.

Dependencies (pinned, downloaded by CMake FetchContent):

- Ogre `v1.10.12`
- MyGUI `MyGUI3.2.2`
- SDL `release-2.30.10`
- OpenAL Soft `1.23.1`

System packages used by that build: a C++14 compiler, CMake, FreeType, FreeImage, ZZip, zlib, and X11/GL headers.

## Linux

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build -j
```

If `c++` is Clang and the link fails with `cannot find -lstdc++`, pass `gcc` and `g++` as above.

`RelWithDebInfo` and `Debug` define `KYBALL_WINDOWED` and `_DEBUG`, so the window stays windowed. Run from the build directory:

```bash
./build/kyball
./build/kyball --touch
```

`--touch` sends the mouse through the same gesture mapper Android uses:

- drag: aim (relative mouse, no button)
- click inside 16 pixels (`TouchGestureMapper::DRAG_THRESHOLD_PX`): left press then release
- Shift held while dragging: second finger, right button held (camera orbit)
- Alt held while dragging: second finger mirrored (pinch / mouse wheel)
- Shift+click twice quickly: right double-click (camera reset)

Without `--touch`, mouse buttons and motion are passed through unchanged.

Game data is `P3DData/` relative to the working directory, or the directory in `KYBALL_DATA` (trailing slash optional). The build writes `P3DData/plugins.cfg` there if it is missing, loading `RenderSystem_GL` and `Plugin_ParticleFX` from the Ogre build. Do not commit extracted meshes, textures, or audio.

Writable files (`ogre.cfg`, logs, `options2.txt`) go to `$XDG_DATA_HOME/KYBALL`, or `~/.local/share/KYBALL` when `XDG_DATA_HOME` is unset.

## Android

The Gradle project is `android/` and compiles the same CMake targets with the NDK (GLES2, SDL2 activity, OpenAL Soft, `/` paths).

```bash
export ANDROID_HOME="$HOME/Android/Sdk"
cd android
gradle assembleDebug
```

Touches go to `TouchGestureMapper` (the Linux `--touch` mapper). There is no second gesture implementation.

Pack game data as APK assets under `android/app/src/main/assets/P3DData/` (the same tree the installer would extract). Do not commit those files. On startup the native code copies `assets/P3DData/` out of the APK into the app's internal storage. `ogre.cfg`, logs, and `options2.txt` are written in that internal directory. Plugins are linked statically (`RenderSystem_GLES2` and `Plugin_ParticleFX`).

The debug APK is `android/app/build/outputs/apk/debug/app-debug.apk` (`arm64-v8a`, `minSdk` 24, `compileSdk` 34, NDK 26.1). FreeImage is off in the Android build, so DDS textures load and PNG/JPG textures do not until an image codec is added.
