#!/bin/sh

mkdir ./build
cd ./build

export Qt6_DIR=~/qt/6.4.2/gcc_64/lib/cmake/Qt6
# export Qt5_DIR=~/qt/5.13.2/gcc_64/lib/cmake/Qt5

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
        -DHSMBUILD_DEBUGGING=ON \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DHSMBUILD_CODECOVERAGE=ON \
        -DCMAKE_TOOLCHAIN_FILE=~/qt/6.4.2/gcc_64/lib/cmake/Qt6/qt.toolchain.cmake \
        ..
    make -j5

    ./tests/hsmUnitTestsSTD
    lcov -c -d . -o ./coverage_std.info
    ./tests/hsmUnitTestsGLib
    lcov -c -d . -o ./coverage_glib.info
    ./tests/hsmUnitTestsGLibmm
    lcov -c -d . -o ./coverage_glibmm.info
    ./tests/hsmUnitTestsQt
    lcov -c -d . -o ./coverage_qt.info

    # disable tests to save time
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DHSMBUILD_TESTS=OFF -DHSMBUILD_VERBOSE=OFF \
          -DHSMBUILD_DISPATCHER_GLIB=ON \
          -DHSMBUILD_DISPATCHER_GLIBMM=ON \
          -DHSMBUILD_DISPATCHER_STD=ON \
          -DHSMBUILD_DISPATCHER_QT=ON \
          -DHSMBUILD_TESTS=OFF \
          -DHSMBUILD_EXAMPLES=ON \
          -DHSMBUILD_DEBUGGING=ON \
          -DHSMBUILD_CODECOVERAGE=ON \
          ..

    cppcheck --addon=../scripts/cppcheck/misra.json \
         --enable=warning,performance,portability,information \
         --inline-suppr \
         --xml \
         --suppressions-list=../scripts/cppcheck/cppcheck_suppress.txt --project=./compile_commands.json \
         -DSTL_AVAILABLE -D__GNU__=1 -D__LITTLE_ENDIAN__ -D__GNUC__

    lcov --add-tracefile ./coverage_std.info \
               -a ./coverage_glib.info \
               -a ./coverage_qt.info \
               -a ./coverage_glibmm.info \
               -o ./coverage.info
    lcov -r ./coverage.info /usr/include/\* . -o ./coverage.info
    lcov -r ./coverage.info \*/build/\* . -o ./coverage.info
    lcov -r ./coverage.info \*/tests/\* . -o ./coverage.info

    genhtml ./coverage.info --output-directory out
fi