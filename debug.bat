pip install "conan>=1.40.3"

mkdir work
cd work
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target install
