#!/bin/sh

mkdir ./build
cd ./build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTS=on -DVERBOSE=off ..
make -j2

./tests/hsmUnitTestsSTD
./tests/hsmUnitTestsGLibmm

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTS=off -DVERBOSE=off ..
cppcheck --enable=warning,performance,portability,information,information --suppressions-list=../etc/cppcheck_suppress.txt --project=compile_commands.json
# --addon=misra.py --rule-texts=???