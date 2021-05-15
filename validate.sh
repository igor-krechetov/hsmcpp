#!/bin/sh

mkdir ./build
cd ./build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DHSMBUILD_TESTS=ON -DHSMBUILD_VERBOSE=OFF ..
make -j2

./tests/hsmUnitTestsSTD
./tests/hsmUnitTestsGLib
./tests/hsmUnitTestsGLibmm
./tests/hsmUnitTestsQt

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DHSMBUILD_TESTS=off -DHSMBUILD_VERBOSE=off ..
cppcheck --enable=warning,performance,portability,information,information --suppressions-list=../etc/cppcheck_suppress.txt --project=compile_commands.json
# --addon=misra.py --rule-texts=???