#!/usr/bin/env bash

build_dir="$1"
install_dir="$2"

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
read -r VERSION < $build_dir/version
