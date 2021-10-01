# Godot Native Example

> **Example project for building C++ GDNative Scripts.** Supports 💰Windows, 🐧Linux & 🍏MacOS (x86_x64/arm64).

## Buildling on Windows

1. Clone this repo.
  ``` sh
  git@github.com:samsface/godot-native-example.git
  ```

2. Download and install:
    - https://cmake.org/download
    - https://visualstudio.microsoft.com/downloads (community edition is fine)
    - https://www.python.org/downloads (download Python3 if you just have 2.7)

3. Generate the Visual Studio project.
    ``` sh
    # in root directory were you cloned this project to, e.g. C:/work/godot-native-example
    debug.bat
    ```
4. Open `godot-native-example.sln` and build. CMake will install the built dlls into the example project for you every time you build. Just open the example project and your good. The library is also reloadable so no need to restart Godot every build.
