#!/usr/bin/env bash
set -euo pipefail

build_dir="$1"

cd $build_dir

echo "Generate code coverage"
lcov --add-tracefile ./coverage_std.info \
    -a ./coverage_glib.info \
    -a ./coverage_qt.info \
    -a ./coverage_glibmm.info \
    -o ./coverage.info

lcov -r ./coverage.info \
        '/usr/include/*' \
        '*/build/*' \
        '*/tests/*' \
        '*/gcc_64/include/QtCore/*' \
        '*/thirdparty/*' \
        -o ./coverage.info

# lcov -r ./coverage.info '/usr/include/*' -o ./coverage.info
# lcov -r ./coverage.info '*/build/*' -o ./coverage.info
# lcov -r ./coverage.info '*/tests/*' -o ./coverage.info
# lcov -r ./coverage.info '*/gcc_64/include/QtCore/*' -o ./coverage.info
# lcov -r ./coverage.info '*/thirdparty/*' -o ./coverage.info

# lcov -r ./coverage.info /usr/include/\* . -o ./coverage.info --ignore-errors empty --ignore-errors unused
# lcov -r ./coverage.info \*/build/\* . -o ./coverage.info --ignore-errors empty --ignore-errors unused
# lcov -r ./coverage.info \*/tests/\* . -o ./coverage.info --ignore-errors empty --ignore-errors unused
# lcov -r ./coverage.info \*/gcc_64/include/QtCore/\* . -o ./coverage.info --ignore-errors empty --ignore-errors unused
# lcov -r ./coverage.info \*/thirdparty/\* . -o ./coverage.info --ignore-errors empty --ignore-errors unused