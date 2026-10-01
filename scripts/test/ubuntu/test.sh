#!/usr/bin/env bash
set -euo pipefail

build_dir="$1"
source_dir="$2"

cd $build_dir

gcc --version
gcov --version
lcov --version

# NOTE: only tokens valid in lcov 1.14 are used here (gcov, source). The test
# CI job runs on ubuntu-22.04, which ships lcov 1.14 - the 2.0-only tokens
# "mismatch" and "version" are rejected with "unknown argument for
# --ignore-errors". "version" is omitted on purpose anyway: a gcov version
# mismatch means the build and test jobs used different GCC major versions and
# must fail loudly, not silently discard coverage. The real guard against that
# is keeping both CI jobs on the same runner image (see build.yml / test.yml).
LCOV_CAPTURE_FLAGS=(--rc geninfo_unexecuted_blocks=1 \
                    --ignore-errors gcov,source)

echo "Run Tests (STD)"
chmod +x ./tests/hsmUnitTestsSTD
timeout 2m ./tests/hsmUnitTestsSTD > ./tests_result_std.log
lcov -c -d . -b "$source_dir" -o ./coverage_std.info "${LCOV_CAPTURE_FLAGS[@]}"

echo "Run Tests (GLib)"
chmod +x ./tests/hsmUnitTestsGLib
timeout 2m ./tests/hsmUnitTestsGLib > ./tests_result_glib.log
lcov -c -d . -b "$source_dir" -o ./coverage_glib.info "${LCOV_CAPTURE_FLAGS[@]}"

echo "Run Tests (GLibmm)"
chmod +x ./tests/hsmUnitTestsGLibmm
timeout 2m ./tests/hsmUnitTestsGLibmm > ./tests_result_glibmm.log
lcov -c -d . -b "$source_dir" -o ./coverage_glibmm.info "${LCOV_CAPTURE_FLAGS[@]}"

echo "Run Tests (Qt)"
chmod +x ./tests/hsmUnitTestsQt
timeout 2m ./tests/hsmUnitTestsQt > ./tests_result_qt.log
lcov -c -d . -b "$source_dir" -o ./coverage_qt.info "${LCOV_CAPTURE_FLAGS[@]}"
