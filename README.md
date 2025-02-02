# SDL_shadercross

This is a library for translating shaders to different formats, intended for use with SDL's GPU API.
It takes SPIRV or HLSL as the source and outputs DXBC, DXIL, SPIRV, MSL, or HLSL.

This library can perform runtime translation and conveniently returns compiled SDL GPU shader objects from HLSL or SPIRV source.
This library also provides a command line interface for offline translation of shaders.

For SPIRV translation, this library depends on SPIRV-Cross: https://github.com/KhronosGroup/SPIRV-Cross
spirv-cross-c-shared.dll (or your platform's equivalent) can be obtained in the Vulkan SDK: https://vulkan.lunarg.com/
For compiling to DXIL, dxcompiler.dll and dxil.dll (or your platform's equivalent) are required.
DXIL dependencies can be obtained here: https://github.com/microsoft/DirectXShaderCompiler/releases
It is strongly recommended that you ship SPIRV-Cross and DXIL dependencies along with your application.
For compiling to DXBC, d3dcompiler_47 is shipped with Windows. Other platforms require vkd3d-utils.

This library is under the zlib license, see LICENSE.txt for details.

## Build Instructions

### Windows

Run these commands from a Visual Studio Developer Command Prompt in order to find the compiler.

#### Build SDL

```
git clone https://github.com/libsdl-org/SDL
cd SDL
cmake -S. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd ..
```

#### Build SDL_shadercross

```
git clone --recursive https://github.com/libsdl-org/SDL_shadercross
cd SDL_shadercross
git submodule update --remote
cmake -S. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release -DSDLSHADERCROSS_VENDORED=ON -DSDL3_DIR=../SDL/build
cmake --build build
```

#### Install locally

```
copy /Y build\shadercross.exe %USERPROFILE%\AppData\Local\Programs
copy /Y ..\SDL\build\SDL3.dll %USERPROFILE%\AppData\Local\Programs
copy /Y build\external\DirectXShaderCompiler\bin\dxcompiler.dll %USERPROFILE%\AppData\Local\Programs
copy /Y build\external\SPIRV-Cross\spirv-cross-c-shared.dll %USERPROFILE%\AppData\Local\Programs
```

### macOS

#### Install spirv-cross

```
brew install spirv-cross
```

#### Build SDL

```
git clone https://github.com/libsdl-org/SDL
cd SDL
cmake -S. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd ..
```

#### Build DirectXShaderCompiler

```
git clone --recursive https://github.com/microsoft/DirectXShaderCompiler
cd DirectXShaderCompiler
cmake -S. -Bbuild -GNinja -Ccmake/caches/PredefinedParams.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd ..
```

#### Build SDL_shadercross

```
git clone https://github.com/libsdl-org/SDL_shadercross
cd SDL_shadercross
```

Change the `CMakeLists.txt` at line 147 from:

```
if(SDLSHADERCROSS_DXC)
    set(DirectXShaderCompiler_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/DirectXShaderCompiler-binaries")
    find_package(DirectXShaderCompiler REQUIRED)
endif()
```

to:

```
if(SDLSHADERCROSS_DXC)
    set(DirectXShaderCompiler_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../DirectXShaderCompiler/build")
    set(DirectXShaderCompiler_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../DirectXShaderCompiler/build/include")
    find_package(DirectXShaderCompiler REQUIRED)
endif()
```

Then configure and build:

```
cmake -S. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release -DSDLSHADERCROSS_SPIRVCROSS_SHARED=OFF -DSDL3_DIR=../SDL/build
cmake --build build
```

#### Install locally:

Note that you _could_ use `shadercross` from the folder you built it from, but it's better to "install" it locally.  These instructions assume you have a `bin` directory in your home directory and that the directory is in your `$PATH`.

```
cp build/shadercross ~/bin
cp ../SDL/build/libSDL3.0.dylib ~/bin
cp ../DirectXShaderCompiler/build/lib/libdxcompiler.dylib ~/bin
```

If you run this now, you will get a `Library not loaded` error as the libraries are defined as loading from `@rpath` and
the binary rpath has not been fixed to use `~/bin`.

View the current `rpath` values in the executable:

```
otool -l ~/bin/shadercross | more 
```

and look for `LC_RPATH` load commands (note that `/Users/username/source/SDL3` will be different for you, depending on where you cloned the git repo):

```
      cmd LC_RPATH
      cmdsize 56
         path /Users/username/source/SDL3/SDL/build (offset 12)
Load command 18
          cmd LC_RPATH
      cmdsize 80
         path /Users/username/source/SDL3/DirectXShaderCompiler/build/lib (offset 12)
```

and then remove the first rpath and change the second to `$HOME/bin`:

```
install_name_tool -delete_rpath /Users/username/source/SDL3/SDL/build ~/bin/shadercross
install_name_tool -rpath /Users/username/source/SDL3/DirectXShaderCompiler/build/lib $HOME/bin ~/bin/shadercross
```
