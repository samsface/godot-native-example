cmake_minimum_required(VERSION 3.2)
project(Veles)

###############################################################################
# options
###############################################################################

OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)

###############################################################################
# conan stuff
###############################################################################

# export godot-cpp into local conan cache
execute_process(COMMAND conan export ${CMAKE_CURRENT_SOURCE_DIR}/lib/godot-cpp local/stable)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.17.0/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                EXPECTED_HASH SHA256=3bef79da16c2e031dc429e1dac87a08b9226418b300ce004cc125a82687baeef
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
 
conan_cmake_configure(REQUIRES godot-cpp/3.3@local/stable taglib/1.13 GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE . BUILD missing SETTINGS ${settings})

find_package(godot-cpp)
find_package(taglib REQUIRED)

###############################################################################

# install build into example project by default
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(
    CMAKE_INSTALL_PREFIX "../app/addons/GDNative" 
    CACHE PATH "..." 
    FORCE)
endif()

# let's us link relative to process on linux
set(CMAKE_INSTALL_RPATH "$ORIGIN")

add_library(Veles src/Godot.cpp)

target_link_libraries(
  Veles
  PRIVATE
    godot-cpp::godot-cpp
    taglib::taglib)

set_target_properties(
  Veles
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON)

if(APPLE)
  set_target_properties(
    Veles
    PROPERTIES 
    OSX_ARCHITECTURES "arm64;x86_64")
endif()

install(TARGETS Veles DESTINATION .)
