#!/bin/sh

cmake -B ./build -DHSMBUILD_TARGET=platformio -DHSMBUILD_PLATFORM=arduino -DHSMBUILD_THREAD_SAFETY=OFF -DHSMBUILD_DISPATCHER_STD=OFF
cmake --build ./build --target install
cd ./build/deploy
pio pkg pack
