#!/bin/sh
export ROOT_07_BUILD=$PWD

cd $ROOT_07_BUILD/using_pkgconfig
rm -Rvf ./build
mkdir ./build
cd ./build
cmake ..
make

cd $ROOT_07_BUILD/using_code
rm -Rvf ./build
rm -Rvf ./hsmcpp
mkdir ./build
mkdir ./hsmcpp
cp -R ../../../include ./hsmcpp/
cp -R ../../../src ./hsmcpp/
cp -R ../../../pkgconfig ./hsmcpp/
cp -R ../../../tools ./hsmcpp/
cp -R ../../../cmake ./hsmcpp/
cp ../../../CMakeLists.txt ./hsmcpp/
cd ./build
cmake ..
make

cd $ROOT_07_BUILD/using_fetch
rm -Rvf ./build
mkdir ./build
cd ./build
cmake ..
make

if command -v conan >/dev/null 2>&1; then
cd $ROOT_07_BUILD/using_conan
rm -Rvf ./build
conan install . --output-folder=build --build=missing
cmake -S . -B ./build -DCMAKE_TOOLCHAIN_FILE=./build/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build ./build
else
echo "conan not found. Skipping using_conan example."
fi
