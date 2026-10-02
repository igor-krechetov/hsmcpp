#!/usr/bin/env python3
"""Extract C++ code and test trace tags into LOBSTER trace files.

Two modes:

  * default (code):  scan C/C++ sources for `// lobster-trace: <namespace> <tag>`
    markers and emit an implementation trace file (schema lobster-imp-trace),
    attaching each marker to the nearest enclosing function above it.

  * --tests:         scan GoogleTest sources for every TEST/TEST_F/TEST_P case
    and emit an activity trace file (schema lobster-act-trace). A test's
    TEST_REQUIREMENTS("<UID>") tags give its requirement links; a TEST_EXCLUDE(
    "reason") marks it intentionally unlinked; a case with neither is still
    emitted (with no link) so unconnected tests surface in the coverage report.
    This reads the SAME macros that lobster-gtest reads at runtime from the
    GTest XML, so the static (no-build) report and the runtime (post-run) report
    come from one single tag with no drift.

Both accept full TRLC UIDs (e.g. HSMCPP.SWR_HSM_040). This is a lightweight,
build-free alternative to the official lobster-cpp (which needs a custom
clang-tidy) and lobster-cpptest (which only accepts CodeBeamer CB-# numeric
IDs, not TRLC UIDs). The code marker syntax matches lobster-cpp's, so it can be
swapped in later with no source changes.

Usage:
    python3 scripts/requirements/extract_cpp_trace.py <out.lobster> <path>...
    python3 scripts/requirements/extract_cpp_trace.py --tests <out.lobster> <path>...
"""
import os
import re
import sys

try:  # LOBSTER 1.x layout
    from lobster.common.items import Implementation, Activity, Tracing_Tag
    from lobster.common.location import File_Reference
    from lobster.common.io import lobster_write
except ImportError:  # LOBSTER 0.x layout
    from lobster.items import Implementation, Activity, Tracing_Tag
    from lobster.location import File_Reference
    from lobster.io import lobster_write

TRACE_RE = re.compile(r"//\s*lobster-trace:\s*(.+)")
EXCLUDE_RE = re.compile(r"//\s*lobster-exclude:\s*(.+)")
# A GTest body tag: TEST_REQUIREMENTS("A") or TEST_REQUIREMENTS("A,B,C") — comma-separated,
# matching how lobster-gtest splits the lobster-tracing property at runtime.
GTEST_TRACE_RE = re.compile(r'TEST_REQUIREMENTS\(\s*"([^"]+)"\s*\)')
# TEST_EXCLUDE("reason") — explicitly declares the test is intentionally not
# linked to any requirement; the reason becomes a LOBSTER justification so the
# test shows as JUSTIFIED (not a gap) in the coverage report.
GTEST_EXCLUDE_RE = re.compile(r'TEST_EXCLUDE\(\s*"([^"]*)"\s*\)')
GTEST_CASE_RE = re.compile(
    r"^\s*(?:TEST|TEST_F|TEST_P|TYPED_TEST)\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)")
# A function/method definition line: "... name(...) ... {" (best-effort).
FUNC_RE = re.compile(
    r"^[A-Za-z_].*?\b([A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z_~][A-Za-z0-9_]*)*)"
    r"\s*\([^;]*$")
# A class / struct definition (enables whole-class linking).
CLASS_RE = re.compile(
    r"^\s*(?:template\s*<[^>]*>\s*)?(class|struct)\s+([A-Za-z_]\w*)\b"
    r"(?!.*;\s*$)")
SRC_EXT = (".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hxx")


def parse_targets(raw):
    """Parse a trace value into (namespace, [tags]).

    Accepts:
      req HSMCPP.SWR_HSM_040
      req HSMCPP.SWR_HSM_040, HSMCPP.SWR_HSM_041
      HSMCPP.SWR_HSM_040                       (namespace defaults to 'req')
    """
    raw = raw.split("//")[0].strip()
    parts = raw.split(None, 1)
    if len(parts) == 2 and not parts[0].startswith(("HSMCPP.", "SYS", "SWR")):
        namespace, rest = parts[0], parts[1]
    else:
        namespace, rest = "req", raw
    tags = [t for t in re.split(r"[,\s]+", rest.strip()) if t]
    return namespace, tags


def iter_files(paths):
    for p in paths:
        if os.path.isfile(p):
            yield p
        elif os.path.isdir(p):
            for root, _, files in os.walk(p):
                for f in files:
                    if os.path.splitext(f)[1] in SRC_EXT:
                        yield os.path.join(root, f)


def nearest_entity(lines, idx):
    """Resolve the entity a `// lobster-trace:` marker at line idx refers to.

    Preference order:
      1. A class/struct or function DEFINITION on the lines immediately below
         the marker (marker precedes the entity) — this is how a whole class or
         a standalone function is tagged.
      2. Otherwise the nearest enclosing function/method/class ABOVE (marker
         sits inside a function body).
    Returns (name, kind, line_no)."""
    n = len(lines)
    # 1. look downward, skipping blank/comment lines, for a definition
    j = idx + 1
    while j < n and (lines[j].strip() == "" or lines[j].lstrip().startswith(("//", "*", "/*"))):
        j += 1
    if j < n:
        cm = CLASS_RE.match(lines[j])
        if cm:
            return cm.group(2), "Class", j + 1
        fm = FUNC_RE.match(lines[j].strip())
        if fm:
            return fm.group(1), "Function", j + 1
    # 2. walk upward to the enclosing entity
    for k in range(idx, -1, -1):
        cm = CLASS_RE.match(lines[k])
        if cm:
            return cm.group(2), "Class", k + 1
        fm = FUNC_RE.match(lines[k].strip())
        if fm:
            return fm.group(1), "Function", k + 1
    return None, "Function", idx + 1


def extract(paths):
    """Emit one Implementation per traced entity. Multiple `// lobster-trace:`
    markers on the same entity — and comma-separated UIDs within one marker —
    are merged into a single item with multiple tracing targets."""
    items = []
    for path in iter_files(paths):
        try:
            lines = open(path, encoding="UTF-8", errors="replace").read().splitlines()
        except OSError:
            continue
        # entity key -> item (dedup so several markers on one entity merge)
        by_entity = {}
        for i, line in enumerate(lines):
            m = TRACE_RE.search(line)
            if not m:
                continue
            namespace, tags = parse_targets(m.group(1))
            if not tags:
                continue
            ename, ekind, eline = nearest_entity(lines, i)
            ename = ename or os.path.basename(path)
            key = "%s:%s:%s" % (path, ename, eline)
            impl = by_entity.get(key)
            if impl is None:
                impl = Implementation(
                    tag      = Tracing_Tag(namespace="cpp",
                                           tag="%s:%s" % (ename, eline)),
                    location = File_Reference(filename=path, line=eline),
                    language = "C++",
                    kind     = ekind,
                    name     = ename,
                )
                by_entity[key] = impl
                items.append(impl)
            for tag in tags:
                impl.add_tracing_target(Tracing_Tag.from_text(namespace, tag))
    return items


def extract_tests(paths):
    """Emit one Activity per GoogleTest case — INCLUDING tests that carry no
    requirement link, so the coverage report surfaces unconnected tests as
    gaps instead of hiding them.

    For each test case:
      * every TEST_REQUIREMENTS("UID,...") in its body adds requirement targets
        (comma-separated UIDs are split, matching lobster-gtest's runtime split);
      * a TEST_EXCLUDE("reason") adds a justification (just_up), so the test
        shows as JUSTIFIED (intentionally not linked) rather than a gap;
      * a case with neither is emitted with no target and no justification, so
        LOBSTER flags it as missing an up-reference (an unconnected test).
    """
    items = []
    for path in iter_files(paths):
        try:
            lines = open(path, encoding="UTF-8", errors="replace").read().splitlines()
        except OSError:
            continue
        # Index every test case first, then attribute each LOBSTER_* tag to the
        # case whose body it falls in (between this case macro and the next).
        cases = [(i, m.group(1), m.group(2))
                 for i, line in enumerate(lines)
                 for m in [GTEST_CASE_RE.match(line)] if m]
        for ci, (idx, suite, test) in enumerate(cases):
            end = cases[ci + 1][0] if ci + 1 < len(cases) else len(lines)
            act = Activity(
                tag       = Tracing_Tag(namespace="gtest",
                                        tag="%s.%s" % (suite, test)),
                location  = File_Reference(filename=path, line=idx + 1),
                framework = "GoogleTest",
                kind      = "Test",
            )
            for j in range(idx, end):
                mt = GTEST_TRACE_RE.search(lines[j])
                if mt:
                    for uid in re.split(r"[,\s]+", mt.group(1).strip()):
                        if uid:
                            act.add_tracing_target(
                                Tracing_Tag.from_text("req", uid))
                me = GTEST_EXCLUDE_RE.search(lines[j])
                if me:
                    reason = me.group(1).strip() or "excluded from requirement tracing"
                    act.just_up.append(reason)
            items.append(act)
    return items


def main():
    args = sys.argv[1:]
    tests_mode = False
    if args and args[0] == "--tests":
        tests_mode = True
        args = args[1:]
    if len(args) < 2:
        sys.exit("usage: extract_cpp_trace.py [--tests] <out.lobster> <path> [path...]")
    out, paths = args[0], args[1:]
    if tests_mode:
        items = extract_tests(paths)
        with open(out, "w", encoding="UTF-8") as fd:
            lobster_write(fd, Activity, "extract_cpp_trace", items)
        print("wrote %d test trace item(s) to %s" % (len(items), out),
              file=sys.stderr)
    else:
        items = extract(paths)
        with open(out, "w", encoding="UTF-8") as fd:
            lobster_write(fd, Implementation, "extract_cpp_trace", items)
        print("wrote %d code trace item(s) to %s" % (len(items), out),
              file=sys.stderr)


if __name__ == "__main__":
    main()
