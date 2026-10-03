BUILD_DIR := "./build"
CMAKE_TOOL := "cmake"

# --------------------------------------------------
# BUILD & RUN
clean:
    #!/usr/bin/env bash
    if [ -d {{BUILD_DIR}} ]; then
        cd {{BUILD_DIR}}
        rm -Rvf ./*
    else
        echo "No build directory found";
    fi

# TODO: use scripts
configure:
    #!/usr/bin/env bash
    set -euxo pipefail
    unset Qt5_DIR
    export PATH=$PATH:~/Qt/6.3.0/gcc_64/bin
    mkdir -p {{BUILD_DIR}}
    cd {{BUILD_DIR}}
    {{CMAKE_TOOL}} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_INSTALL_PREFIX=/usr ..

# TODO: use scripts
build: configure
    #!/usr/bin/env bash
    set -euxo pipefail
    mkdir -p {{BUILD_DIR}}
    cd {{BUILD_DIR}}
    clear
    make -j5
    pwd

run:
    {{BUILD_DIR}}/src/hsm_ide

build_run: build run

clean_build: clean build

# --------------------------------------------------
# VALIDATION
test: build
    ctest --test-dir {{BUILD_DIR}}

list_tests:
    ctest --test-dir {{BUILD_DIR}} -N

list_test_usecases testname:
    {{BUILD_DIR}}/tests/{{testname}} -functions

one_test testname: build
    ctest --test-dir {{BUILD_DIR}} -R {{testname}}

test_usecase testname usecase:
    {{BUILD_DIR}}/tests/{{testname}} {{usecase}} -maxwarnings 100000

# --------------------------------------------------
# REQUIREMENTS

# Validate the TRLC requirements only (fast, CI-friendly).
req_validate:
    trlc --brief requirements/

# Build the requirements documentation.
#   just req_build         requirements only (validate TRLC + render the doc)
#   just req_build full    also add STATIC code + test traceability columns
#                          (runs doxygen and the source tag scan)
req_build mode="req":
    #!/usr/bin/env bash
    set -euo pipefail
    OUT={{BUILD_DIR}}/requirements
    mkdir -p "$OUT"
    # Validate the TRLC model + requirements.
    trlc requirements/
    if [ "{{mode}}" = "full" ]; then
        # STATIC traceability for the DOC: doxygen resolves the code entities
        # carrying @requirement, and a source scan reads the TEST_REQUIREMENTS test
        # tags. Both are design intent (no execution) — exactly what the doc
        # reflects. Execution evidence is `just req_trace` (LOBSTER report).
        doxygen config/lobster/Doxyfile
        python3 scripts/requirements/doxygen_to_lobster.py --all {{BUILD_DIR}}/doxygen/xml "$OUT/code.lobster"
        python3 scripts/requirements/extract_cpp_trace.py --tests "$OUT/tests_static.lobster" tests
        python3 scripts/requirements/render_html.py "$OUT/hsmcpp_requirements.html"
    else
        # Requirements only — no code/test columns.
        python3 scripts/requirements/render_html.py --no-trace "$OUT/hsmcpp_requirements.html"
    fi

req_coverage: (req_build "full") test_trace
    #!/usr/bin/env bash
    set -euo pipefail
    OUT={{BUILD_DIR}}/requirements
    mkdir -p "$OUT"
    # 1. Extract requirement trace data (SYS + SW) from the TRLC sources.
    lobster-trlc --config config/lobster/lobster-trlc.yaml --out "$OUT/reqs.lobster" \
        requirements/model/hsmcpp.rsl \
        requirements/system_requirements.trlc \
        requirements/software_requirements.trlc \
        requirements/terminology.trlc
    # 2. LOBSTER 1.0.6 dropped the "with kind" source filter, so split the
    #    combined requirements file into per-level files by item kind.
    python3 scripts/requirements/split_reqs.py "$OUT/reqs.lobster" \
        "$OUT/sys.lobster" "$OUT/sw.lobster"
    # 3. code.lobster is produced by req_build (doxygen --all); reuse it here.
    # 4. Test evidence for the coverage report. `just req_trace` writes an
    #    EXECUTED tests.lobster (runtime proof). Absent that, fall back to the
    #    STATIC test links so the report still renders (design intent only).
    if [ ! -f "$OUT/tests.lobster" ]; then
        cp "$OUT/tests_static.lobster" "$OUT/tests.lobster"
    fi
    # 5. Build the traceability report and render HTML.
    cp config/lobster/lobster.conf "$OUT/lobster.conf"
    ( cd "$OUT" && lobster-report --lobster-config lobster.conf --out report.lobster \
        && lobster-html-report report.lobster --out hsmcpp_coverage.html )
    echo ""
    echo "Coverage report:  $OUT/hsmcpp_coverage.html"
    echo "Requirement doc:  $OUT/hsmcpp_requirements.html"



# --------------------------------------------------
# INTERNAL

# Runtime coverage with EXECUTION EVIDENCE: build + run the suite, then read the
# TEST_REQUIREMENTS tags from the GTest XML (proof the test actually ran) via
# lobster-gtest, and feed that into the same report.
test_trace: build
    #!/usr/bin/env bash
    set -euo pipefail
    OUT={{BUILD_DIR}}/requirements
    mkdir -p "$OUT"

    # Produce GoogleTest's NATIVE xml per dispatcher binary (preserves the
    # TEST_REQUIREMENTS RecordProperty entries that `ctest --output-junit` drops). Driven
    # through ctest via the per-binary `<target>_trace` tests registered in
    # tests/CMakeLists.txt, so ctest remains the test driver.
    mkdir -p {{BUILD_DIR}}/gtest-xml
    rm -f {{BUILD_DIR}}/gtest-xml/*.xml
    # Run only the native-XML trace tests (one per dispatcher binary).
    ctest --test-dir {{BUILD_DIR}} -R '_trace$' --output-on-failure

    # Merge the per-dispatcher native GTest XMLs into ONE deduplicated test trace,
    # keyed on the test's source file:line so a test compiled into every
    # dispatcher binary is linked to its requirements exactly once (the dispatcher
    # list is preserved on the item).
    python3 scripts/requirements/merge_gtest_trace.py "$OUT/tests.lobster" {{BUILD_DIR}}/gtest-xml