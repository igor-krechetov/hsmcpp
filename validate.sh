#!/bin/sh

mkdir ./build
cd ./build

# export Qt6_DIR=~/Qt/6.3.0/gcc_64/lib/cmake/Qt6
export Qt5_DIR=~/qt/5.13.2/gcc_64/lib/cmake/Qt5

if [ ${PWD##*/} = "build" ]
then
    rm -Rvf ./*
    cmake -DHSMBUILD_VERBOSE=OFF \
        -DHSMBUILD_DISPATCHER_GLIB=ON \
        -DHSMBUILD_DISPATCHER_GLIBMM=ON \
        -DHSMBUILD_DISPATCHER_STD=ON \
        -DHSMBUILD_DISPATCHER_QT=ON \
        -DHSMBUILD_TESTS=ON \
        -DHSMBUILD_EXAMPLES=ON \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        ..
        # -DCMAKE_TOOLCHAIN_FILE=~/Qt/6.3.0/gcc_64/lib/cmake/Qt6/qt.toolchain.cmake \
        ##..
    make -j5

    ./tests/hsmUnitTestsSTD
    ./tests/hsmUnitTestsGLib
    ./tests/hsmUnitTestsGLibmm
    ./tests/hsmUnitTestsQt

    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DHSMBUILD_TESTS=OFF -DHSMBUILD_VERBOSE=OFF ..
    cppcheck --enable=warning,performance,portability,information,information --suppressions-list=../etc/cppcheck_suppress.txt --project=compile_commands.json
    # --addon=misra.py --rule-texts=???
fi
