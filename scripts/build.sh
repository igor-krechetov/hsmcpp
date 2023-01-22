#!/bin/sh

mkdir ./build
cd ./build
cmake -DHSMBUILD_TESTS=ON -DHSMBUILD_VERBOSE=OFF -DHSMBUILD_DISPATCHER_QT=OFF -DHSMBUILD_DISPATCHER_GLIB=OFF -DHSMBUILD_DISPATCHER_GLIBMM=OFF ..
make -j5