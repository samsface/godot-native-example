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
 
3. Open `godot-native-example.sln` and build. That's it. Run the godot project and it should print "ciao" to the output.


## For all builds

- CMake will install the plugin into the example project directory every time you build so need to copy any files into the Godot project.
- You DO NOT need to restart Godot every time you rebuild. Godot will reload the plugin.
