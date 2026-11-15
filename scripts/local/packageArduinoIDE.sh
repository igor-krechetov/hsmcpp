#!/bin/sh

cmake -B ./build -DHSMBUILD_TARGET=arduinoide -DHSMBUILD_PLATFORM=arduino -DHSMBUILD_THREAD_SAFETY=OFF -DHSMBUILD_DISPATCHER_STD=OFF
cmake --build ./build --target install

# validate package
# arduino-lint --compliance strict --recursive --library-manager submit ./build/deploy/arduinoide
