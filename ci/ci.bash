#!/bin/bash

mkdir -p work
cd work

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install/lib

unameOut="$(uname -s)"
case "${unameOut}" in
    CYGWIN*)    machine=windows;;
    MINGW*)     machine=windows;;
    *)          machine="${unameOut}"
esac

if [ "${machine}" = "windows" ]; then
  cmake --build . --config Release --target install
else
  cmake --build . --config Release --target install/strip
fi
