#!/bin/sh
rm -Rvf ./build
rm -Rvf ./hsmcpp
mkdir ./build
mkdir ./hsmcpp
cp -R ../../cmake ./hsmcpp/
cp -R ../../include ./hsmcpp/
cp -R ../../src ./hsmcpp/
cp -R ../../pkgconfig ./hsmcpp/
cp -R ../../tools ./hsmcpp/
cp ../../CMakeLists.txt ./hsmcpp/
cd ./build
cmake ..
make
