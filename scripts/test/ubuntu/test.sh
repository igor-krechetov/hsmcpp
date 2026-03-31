#!/usr/bin/env bash
set -euo pipefail

build_dir="$1"

cd $build_dir

echo "Run Tests (STD)"
chmod +x ./tests/hsmUnitTestsSTD
timeout 2m ./tests/hsmUnitTestsSTD > ./tests_result_std.log
lcov -c -d . -o ./coverage_std.info --ignore-errors mismatch

echo "Run Tests (GLib)"
chmod +x ./tests/hsmUnitTestsGLib
timeout 2m ./tests/hsmUnitTestsGLib > ./tests_result_glib.log
lcov -c -d . -o ./coverage_glib.info --ignore-errors mismatch

echo "Run Tests (GLibmm)"
chmod +x ./tests/hsmUnitTestsGLibmm
timeout 2m ./tests/hsmUnitTestsGLibmm > ./tests_result_glibmm.log
lcov -c -d . -o ./coverage_glibmm.info --ignore-errors mismatch

echo "Run Tests (Qt)"
chmod +x ./tests/hsmUnitTestsQt
timeout 2m ./tests/hsmUnitTestsQt > ./tests_result_qt.log
lcov -c -d . -o ./coverage_qt.info --ignore-errors mismatch