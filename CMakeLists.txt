cmake_minimum_required(VERSION 3.10)
project(gdnative-example)

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
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_configure(REQUIRES godot-cpp/3.3@local/stable GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE . BUILD missing SETTINGS ${settings})

find_package(godot-cpp)
find_package(GIF)

###############################################################################

# install build into example project by default
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(
    CMAKE_INSTALL_PREFIX "../app/addons/example" 
    CACHE PATH "..." 
    FORCE)
endif()

# let's us link relative to process on linux
set(CMAKE_INSTALL_RPATH "$ORIGIN")

add_library(gdnative-example src/Godot.cpp)

target_link_libraries(
  gdnative-example
  PRIVATE
    godot-cpp::godot-cpp)

set_target_properties(
  gdnative-example
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON)

if(APPLE)
  set_target_properties(
    gdnative-example
    PROPERTIES 
    OSX_ARCHITECTURES "arm64;x86_64")
endif()

install(TARGETS gdnative-example DESTINATION .)
