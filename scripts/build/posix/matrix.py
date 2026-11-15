#!/usr/bin/env python3
"""Builds and tests every configuration listed in matrix.yaml (POSIX platform).

Every configuration is built in its own directory. A failure does not stop the run: all selected
configurations are executed, a summary table is printed (and written to the GitHub step summary) and
the exit code is 1 if any of them failed.

Requires PyYAML (pip install pyyaml). See matrix.yaml for the file format.
"""
import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

try:
    import yaml
except ImportError:
    sys.exit("PyYAML is required: python3 -m pip install pyyaml")

SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_SRC_DIR = SCRIPT_DIR.parents[2]
BUILD_TYPES = ("Debug", "Release")
GROUP_KEYS = {"build_type", "options", "configs"}
CONFIG_KEYS = {"name", "build_type", "options", "expect_failure"}

IN_CI = bool(os.environ.get("GITHUB_ACTIONS"))


class MatrixError(Exception):
    """Invalid matrix definition."""


class Config:
    def __init__(self, name, group, build_type, options, expect_failure):
        self.name = name
        self.group = group
        self.build_type = build_type
        self.options = options
        self.expect_failure = expect_failure

    def cmake_args(self):
        # tests ON / examples OFF first, so a configuration can override them
        opts = {"HSMBUILD_EXAMPLES": "OFF", "HSMBUILD_TESTS": "ON"}
        opts.update(self.options)
        return [f"-DCMAKE_BUILD_TYPE={self.build_type}"] + [f"-D{k}={v}" for k, v in opts.items()]

    @property
    def runs_tests(self):
        return self.options.get("HSMBUILD_TESTS", "ON") != "OFF"

    @property
    def expectation(self):
        return "configure fails" if self.expect_failure else "pass"


def option_value(value):
    """YAML turns unquoted ON/OFF into booleans, CMake needs them back."""
    if isinstance(value, bool):
        return "ON" if value else "OFF"
    return str(value)


def load_matrix(path):
    with open(path, encoding="utf-8") as f:
        data = yaml.safe_load(f)

    if not isinstance(data, dict) or not isinstance(data.get("groups"), dict):
        raise MatrixError("top level 'groups' mapping is required")

    configs, names = [], set()
    for group, gdef in data["groups"].items():
        if not isinstance(gdef, dict) or not isinstance(gdef.get("configs"), list):
            raise MatrixError(f"group '{group}': 'configs' list is required")
        unknown = set(gdef) - GROUP_KEYS
        if unknown:
            raise MatrixError(f"group '{group}': unknown keys {sorted(unknown)}")

        group_type = gdef.get("build_type", "Debug")
        group_opts = {k: option_value(v) for k, v in (gdef.get("options") or {}).items()}

        for cdef in gdef["configs"]:
            unknown = set(cdef) - CONFIG_KEYS
            if unknown:
                raise MatrixError(f"group '{group}': unknown keys {sorted(unknown)} in {cdef.get('name')}")
            name = cdef.get("name")
            if not name:
                raise MatrixError(f"group '{group}': configuration without a name")
            if name in names:
                raise MatrixError(f"duplicate configuration name '{name}'")
            names.add(name)

            build_type = cdef.get("build_type", group_type)
            if build_type not in BUILD_TYPES:
                raise MatrixError(f"{name}: build_type must be one of {BUILD_TYPES}")

            options = dict(group_opts)
            options.update({k: option_value(v) for k, v in (cdef.get("options") or {}).items()})

            expect_failure = cdef.get("expect_failure")
            if expect_failure is not None:
                re.compile(expect_failure)  # fail early on an invalid regex

            configs.append(Config(name, group, build_type, options, expect_failure))
    return configs


def ci_group_start(title):
    print(f"::group::{title}" if IN_CI else f"=== {title} ===", flush=True)


def ci_group_end():
    if IN_CI:
        print("::endgroup::", flush=True)


def run_logged(cmd, log_path):
    """Runs a command, streaming its output to the console and to log_path. Returns (code, output)."""
    lines = []
    with open(log_path, "w", encoding="utf-8") as log:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, errors="replace")
        for line in proc.stdout:
            sys.stdout.write(line)
            log.write(line)
            lines.append(line)
        proc.wait()
    sys.stdout.flush()
    return proc.returncode, "".join(lines)


def run_config(cfg, src_dir, build_root):
    """Returns None on success or the name of the failed stage."""
    cfg_dir = build_root / cfg.name
    out_dir = cfg_dir / "out"
    cfg_dir.mkdir(parents=True, exist_ok=True)

    code, output = run_logged(["cmake", "-S", str(src_dir), "-B", str(out_dir)] + cfg.cmake_args(),
                              cfg_dir / "configure.log")

    if cfg.expect_failure:
        if code == 0:
            print("Configuration was expected to fail, but succeeded")
            return "configure (expected to fail)"
        if not re.search(cfg.expect_failure, re.sub(r"\s+", " ", output)):
            print(f"Configuration failed, but the output does not match: {cfg.expect_failure}")
            return "configure (unexpected error)"
        return None

    if code != 0:
        return "configure"

    code, _ = run_logged(["cmake", "--build", str(out_dir), "--parallel", str(os.cpu_count() or 2)],
                         cfg_dir / "build.log")
    if code != 0:
        return "build"

    if cfg.runs_tests:
        # <target>_trace runs a whole test binary once (see tests/CMakeLists.txt)
        code, _ = run_logged(["ctest", "--test-dir", str(out_dir), "-R", "_trace$", "--no-tests=error",
                              "--timeout", "600", "--output-on-failure"], cfg_dir / "test.log")
        if code != 0:
            return "test"
    return None


def write_summary(results, group_filter):
    lines = [f"### Build matrix{f' ({group_filter})' if group_filter else ''}", "",
             "| Configuration | Result | Failed stage |", "|---|---|---|"]
    lines += [f"| {cfg.name} | {'PASS' if stage is None else 'FAIL'} | {stage or '-'} |"
              for cfg, stage in results]
    path = os.environ.get("GITHUB_STEP_SUMMARY")
    if path:
        with open(path, "a", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--group", help="run only configurations of this group")
    parser.add_argument("--only", help="run only this configuration")
    parser.add_argument("--list", action="store_true", help="print selected configurations and exit")
    parser.add_argument("--src-dir", type=Path, default=DEFAULT_SRC_DIR, help="hsmcpp sources")
    parser.add_argument("--build-root", type=Path, help="where build dirs are created (default: <src-dir>/build-matrix)")
    parser.add_argument("--matrix", type=Path, default=SCRIPT_DIR / "matrix.yaml", help="matrix definition")
    args = parser.parse_args()

    try:
        configs = load_matrix(args.matrix)
    except (MatrixError, re.error, yaml.YAMLError, OSError) as e:
        sys.exit(f"Invalid matrix {args.matrix}: {e}")

    selected = [c for c in configs
                if (not args.group or c.group == args.group) and (not args.only or c.name == args.only)]
    if not selected:
        sys.exit(f"No configurations selected (group={args.group!r}, only={args.only!r})")

    if args.list:
        for c in selected:
            print(f"{c.name:<22} {c.group:<18} {c.build_type:<8} {c.expectation}")
        return 0

    src_dir = args.src_dir.resolve()
    build_root = (args.build_root or src_dir / "build-matrix").resolve()

    results = []
    for cfg in selected:
        ci_group_start(f"{cfg.name}  [{cfg.group}, {cfg.build_type}, {cfg.expectation}]")
        print("cmake args:", " ".join(cfg.cmake_args()), flush=True)
        stage = run_config(cfg, src_dir, build_root)
        ci_group_end()
        if stage and IN_CI:
            print(f"::error title=Matrix configuration failed::{cfg.name} failed at stage: {stage}")
        results.append((cfg, stage))

    print("\nMatrix summary:")
    for cfg, stage in results:
        print(f"  {cfg.name:<24} {'PASS' if stage is None else 'FAIL':<5} {stage or '-'}")
    write_summary(results, args.group)

    failed = [cfg.name for cfg, stage in results if stage]
    if failed:
        print(f"{len(failed)} of {len(results)} configuration(s) failed: {', '.join(failed)}", file=sys.stderr)
        return 1
    print(f"All {len(results)} configuration(s) passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
