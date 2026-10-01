#!/usr/bin/env python3
"""Merge per-dispatcher GoogleTest XML into ONE deduplicated test trace file.

The hsmcpp unit tests are compiled into several dispatcher-specific binaries
(hsmUnitTestsSTD, hsmUnitTestsGLib, hsmUnitTestsGLibmm, hsmUnitTestsQt,
hsmUnitTestsFreeRTOS) from the SAME test sources, so each tagged test runs once
per dispatcher. For requirement traceability we want each logical test linked
exactly ONCE, not five times.

This reads GoogleTest's native XML from every dispatcher binary, and merges the
test cases by their source identity — the `lobster-tracing-file` + line the
LOBSTER_TRACE macro records. The merged activity keeps:
  * the union of requirement UIDs,
  * the list of dispatchers that ran it (in the item text / name),
  * overall status = ok only if it passed under every dispatcher.

A test whose source line is dispatcher-specific (guarded / distinct line) has a
unique file:line, so it is naturally NOT merged with others.

Usage:
    python3 merge_gtest_trace.py <out.lobster> <xml_dir_or_files>...
"""
import glob
import os
import re
import sys
import xml.etree.ElementTree as ET

try:  # LOBSTER 1.x
    from lobster.common.items import Activity, Tracing_Tag
    from lobster.common.location import File_Reference, Void_Reference
    from lobster.common.io import lobster_write
except ImportError:  # LOBSTER 0.x
    from lobster.items import Activity, Tracing_Tag
    from lobster.location import File_Reference, Void_Reference
    from lobster.io import lobster_write

UID_SPLIT = re.compile(r"[,\s]+")


def dispatcher_of(xml_path):
    """Derive a dispatcher label from the binary/xml name, e.g.
    hsmUnitTestsSTD.xml -> STD."""
    base = os.path.splitext(os.path.basename(xml_path))[0]
    m = re.search(r"hsmUnitTests(.+)", base)
    return m.group(1) if m else base


def iter_xml(paths):
    for p in paths:
        if os.path.isdir(p):
            yield from sorted(glob.glob(os.path.join(p, "*.xml")))
        else:
            yield p


def parse_testcases(xml_path):
    """Yield (suite, name, file, line, uids, passed) for every test case that
    carries a lobster-tracing property."""
    try:
        root = ET.parse(xml_path).getroot()
    except (ET.ParseError, OSError):
        return
    for tc in root.iter("testcase"):
        suite = tc.get("classname") or ""
        name = tc.get("name") or ""
        passed = tc.find("failure") is None
        uids, src_file, src_line = [], None, None
        for props in tc.iter("property"):
            n, v = props.get("name"), props.get("value")
            if n == "lobster-tracing":
                uids += [u for u in UID_SPLIT.split((v or "").strip()) if u]
            elif n == "lobster-tracing-file":
                src_file = v
            elif n == "lobster-tracing-line":
                src_line = v
        if uids:
            yield suite, name, src_file, src_line, uids, passed


def main():
    if len(sys.argv) < 3:
        sys.exit("usage: merge_gtest_trace.py <out.lobster> <xml_dir_or_files>...")
    out, inputs = sys.argv[1], sys.argv[2:]

    merged = {}  # (file, line) -> record
    for xml_path in iter_xml(inputs):
        disp = dispatcher_of(xml_path)
        for suite, name, src_file, src_line, uids, passed in parse_testcases(xml_path):
            key = (src_file, src_line)
            rec = merged.get(key)
            if rec is None:
                rec = {
                    "suite": suite, "name": name,
                    "file": src_file, "line": src_line,
                    "uids": list(uids),
                    "dispatchers": {}, "all_pass": True,
                }
                merged[key] = rec
            # union UIDs (preserve order, dedup)
            for u in uids:
                if u not in rec["uids"]:
                    rec["uids"].append(u)
            rec["dispatchers"][disp] = passed
            rec["all_pass"] = rec["all_pass"] and passed

    items = []
    for rec in merged.values():
        if rec["file"]:
            try:
                loc = File_Reference(filename=rec["file"],
                                     line=int(rec["line"]) if rec["line"] else None)
            except (TypeError, ValueError):
                loc = File_Reference(filename=rec["file"])
        else:
            loc = Void_Reference()
        disp_list = ", ".join(sorted(rec["dispatchers"]))
        act = Activity(
            tag       = Tracing_Tag(namespace="gtest",
                                    tag="%s.%s" % (rec["suite"], rec["name"])),
            location  = loc,
            framework = "GoogleTest",
            kind      = "Test",
            status    = "ok" if rec["all_pass"] else "fail",
        )
        act.text = "Dispatchers: %s" % disp_list
        for u in rec["uids"]:
            act.add_tracing_target(Tracing_Tag.from_text("req", u))
        items.append(act)

    with open(out, "w", encoding="UTF-8") as fd:
        lobster_write(fd, Activity, "merge_gtest_trace", items)
    print("merged %d dispatcher XML(s) -> %d unique tagged test(s) in %s"
          % (len(list(iter_xml(inputs))), len(items), out), file=sys.stderr)


if __name__ == "__main__":
    main()
