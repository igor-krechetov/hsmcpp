#!/bin/sh

cmake -B ./build -DHSMBUILD_TARGET=package -DHSMBUILD_PLATFORM=arduino
cmake --build ./build --target install
cd ./build/deploy
pio pkg pack
