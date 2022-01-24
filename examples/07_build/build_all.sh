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