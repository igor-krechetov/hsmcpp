#!/usr/bin/env bash
set -euo pipefail

build_dir="$1"
build_type="$2"
cmake_args="$3"
src_dir="$4"
build_tests="$5"
build_examples="$6"

cmake -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type" \
  -DHSMBUILD_VERBOSE=OFF \
  -DHSMBUILD_PLATFORM=posix \
  -DHSMBUILD_DISPATCHER_GLIB=ON \
  -DHSMBUILD_DISPATCHER_GLIBMM=ON \
  -DHSMBUILD_DISPATCHER_STD=ON \
  -DHSMBUILD_DISPATCHER_QT=ON \
  -DHSMBUILD_EXAMPLES="$build_examples" \
  -DHSMBUILD_TESTS="$build_tests" \
  -DHSMBUILD_CODECOVERAGE=ON \
  -DHSMBUILD_DEBUGGING=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  $cmake_args \
  "$src_dir"
