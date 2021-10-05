# Godot Native Example

In this project:
- **An example C++ GDNative plugin.** 
- **Instructions to build the plugin including what software you'll need.**
- **Support for 💰Windows, 🐧Linux & 🍏MacOS (x86_x64/arm64).**
- **🤖 An automated build pipeline to build for all above platforms.**

## Buildling on Windows

1. Download and install:
    - https://cmake.org/download
    - https://visualstudio.microsoft.com/downloads (community edition is fine)
    - https://www.python.org/downloads **(must use Python 3!)**

2. Clone this repo and run debug.bat.
  ``` bat
  git clone git@github.com:samsface/godot-native-example.git
  cd godot-native-example
  debug.bat
  ```
 
3. Open `godot-native-example.sln` and build. 

4. Run the godot project in the app directory of this repo. If it prints "ciao" to the output, it worked!

## For all builds

- You DO NOT need to copy and built files into the Godot project. CMake does this for you.
- You DO NOT need to restart Godot every time you rebuild. Godot will reload the plugin.

## Using the CI:

This project contains a github actions file that will build your project for all supported platforms every time you push changes. To use it, just Fork this project and then just push changes. You'll see the build status in the Actions tab of your fork.
