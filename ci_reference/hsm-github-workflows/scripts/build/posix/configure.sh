#!/usr/bin/env bash
set -euo pipefail

build_dir="$1"
build_type="$2"
cmake_args="$3"
src_dir="$4"
build_tests="$5"

cmake -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DHSMBUILD_TESTS="$build_tests" \
  $cmake_args \
  "$src_dir"
