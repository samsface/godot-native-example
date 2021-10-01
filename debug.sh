#!/bin/bash

pip install "conan>=1.40.3"

mkdir -p work
cd work
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target install
