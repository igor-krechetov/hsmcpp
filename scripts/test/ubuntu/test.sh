#!/usr/bin/env bash
set -euo pipefail

build_dir="$1"

cd $build_dir

echo "Run Tests (STD)"
timeout 2m ./tests/hsmUnitTestsSTD > ./tests_result_std.log
lcov -c -d . -o ./coverage_std.info

echo "Run Tests (GLib)"
timeout 2m ./tests/hsmUnitTestsGLib > ./tests_result_glib.log
lcov -c -d . -o ./coverage_glib.info

echo "Run Tests (GLibmm)"
timeout 2m ./tests/hsmUnitTestsGLibmm > ./tests_result_glibmm.log
lcov -c -d . -o ./coverage_glibmm.info

echo "Run Tests (Qt)"
timeout 2m ./tests/hsmUnitTestsQt > ./tests_result_qt.log
lcov -c -d . -o ./coverage_qt.info