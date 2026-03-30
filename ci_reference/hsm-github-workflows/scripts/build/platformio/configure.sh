#!/usr/bin/env bash
set -euo pipefail

build_dir="$1"
build_type="$2"
cmake_args="$3"
src_dir="$4"
build_tests="$5"

cmake -S "$src_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type" \
  -DHSMBUILD_TARGET=platformio -DHSMBUILD_PLATFORM=arduino -DHSMBUILD_TESTS="$build_tests" \
  $cmake_args
