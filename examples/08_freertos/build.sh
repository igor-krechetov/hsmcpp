#!/bin/sh
rm -Rvf ./build
rm -Rvf ./hsmcpp
mkdir ./build
mkdir ./hsmcpp
ln -s ../../../cmake ./hsmcpp/cmake
ln -s ../../../include ./hsmcpp/include
ln -s ../../../src ./hsmcpp/src
ln -s ../../../pkgconfig ./hsmcpp/pkgconfig
ln -s ../../../tools ./hsmcpp/tools
ln -s ../../../CMakeLists.txt ./hsmcpp/CMakeLists.txt
ln -s ../../../version.in ./hsmcpp/version.in
cd ./build
cmake -DFREERTOS_ROOT=$1 ..
make
