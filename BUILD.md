# Building Kyball

Linux and Android builds use the CMake targets in the repository root. Ogre **1.10.12** (not Ogre-Next) is fetched and built with the legacy OpenGL render system on Linux and GLES2 on Android. `Kyball_Installer.exe` (Inno Setup 5.5.7) was extracted and its `P3DData` tree is the gitignored data directory; the installer's `plugins.cfg` was removed from that copy so the Linux build can write `RenderSystem_GL`. `OgreMain.dll` in the install root has no version resource and no version in the filename. Its mesh serializers are `[MeshSerializer_v1.40]` through `[MeshSerializer_v1.10]`, which is the Ogre 1.6.x set, and it links the VC90 CRT (`msvcr90.dll`). `Plugin_ParticleFX.dll` sits beside it and also has no version resource. There is no `RenderSystem_*.dll` in the install root; `RenderSystem_Direct3D9.dll` and `RenderSystem_GL.dll` are under `P3DData/plugins`, likewise with no version resource. `cg.dll` is NVIDIA Cg 2.0.0.10. Materials are 21 `.material` scripts and 412 materials. Fifteen of those scripts (per-cosmos scenes, `gameplay.material` particles such as `Ambient/Droplet`, skyboxes, and `OgreCore.material` overlays) are fixed-function (`texture_unit`, `ambient` / `diffuse`) and do not name a vertex or fragment program. `general.material` is 111 materials, of which 108 inherit `base_material`, whose passes reference the Cg programs `ambient_vs`, `diffuse_vs`, `ambient_ps`, and `diffuse_ps` (`general.program` / `general.cg`). Five compositor materials in `programs.zip` (`Bloom.material`, `Bloom2.material`, `hdr.material`, `SharpenEdges.material`, `RadialBlur.material`) name `vertex_program` / `fragment_program` in GLSL, HLSL, and Cg (16 Cg, 15 GLSL, 15 HLSL, and 12 `unified` program declarations in the script set). The game enables the `Bloom`, `BloomGame`, and `Sharpen Edges` compositors. MyGUI layouts are 16 files whose root element is `<MyGUI type="Layout">` with no version attribute. Resource scripts use `version="1.1"` and `core_layer.xml` uses `version="1.2"`. MyGUI 3.2.2 loads that shape: its layout reader deserializes as version 1.0.0 and still registers the `Edit`, `StaticImage`, `StaticText`, and `HScroll` factories. No layout here carries a version newer than 3.2. `snd/` contains 239 `.ogg` files and 0 `.wav` files. `resources.cfg` uses backslashes and locations relative to `P3DData` (`Zip=gfx\general.zip`, `FileSystem=gfx\unref`), plus `FileSystem=..\branding`. Ogre 1.10.12 and MyGUI 3.2.2 stay pinned. The fixed-function scene scripts and these MyGUI layouts are what that pin is for. The GLSL/HLSL compositor scripts and the Ogre 1.6.x DLL are recorded here, and this step does not retarget the engine.

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

Run `./build/kyball` from the repository root so the working directory contains `P3DData/`. Game data is that directory, or the directory in `KYBALL_DATA` (trailing slash optional). `resources.cfg` uses Windows backslashes (`Zip=gfx\general.zip`) and `FileSystem=..\branding`; the loader rewrites `\` to `/` and prefixes `P3DData/`, so `branding/` must sit next to `P3DData/`. Copy it from the installer (`app/branding`) and do not commit it. The build writes `P3DData/plugins.cfg` there if it is missing, loading `RenderSystem_GL` and `Plugin_ParticleFX` from the Ogre build. Do not commit extracted meshes, textures, or audio.

The installer ships effect sounds as `.ogg` only. Linux and Android load `.ogg`. A Windows debug build still looks for `.wav`. Lighting materials reference Cg programs and fall back to their fixed-function technique, because this build does not link the Cg plugin. Compositor materials include a GLSL technique after the HLSL one.

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
