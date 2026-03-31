#!/usr/bin/env bash
set -euo pipefail

build_dir="$1"
build_type="$2"

cmake --build "$build_dir" --parallel 2 --config "$build_type"
