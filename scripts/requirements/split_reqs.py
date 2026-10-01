#!/usr/bin/env python3
"""Split a combined lobster-req-trace file into per-kind files.

LOBSTER 1.0.6 removed the `source: "..." with kind "..."` filter from the
tracing policy, so System and Software requirements can no longer be separated
from a single reqs.lobster inside lobster.conf. This helper splits the combined
lobster-trlc output into one file per requirement kind, preserving the schema.

Usage:
    python3 split_reqs.py <reqs.lobster> <sys.lobster> <sw.lobster>
"""
import json
import sys


def main():
    if len(sys.argv) != 4:
        sys.exit("usage: split_reqs.py <reqs.lobster> <sys.lobster> <sw.lobster>")
    src, sys_out, sw_out = sys.argv[1:]
    doc = json.loads(open(src, encoding="UTF-8").read())
    data = doc.get("data", [])

    def emit(path, kind):
        out = dict(doc)
        out["data"] = [i for i in data if i.get("kind") == kind]
        with open(path, "w", encoding="UTF-8") as fd:
            json.dump(out, fd, indent=2)
            fd.write("\n")
        return len(out["data"])

    n_sys = emit(sys_out, "System_Requirement")
    n_sw = emit(sw_out, "Software_Requirement")
    print("split: %d System_Requirement -> %s, %d Software_Requirement -> %s"
          % (n_sys, sys_out, n_sw, sw_out), file=sys.stderr)


if __name__ == "__main__":
    main()
