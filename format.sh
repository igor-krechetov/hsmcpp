#!/bin/bash

# Usage:
# ./format-all.sh [project_dir] [clang_format_config]

# Set the path to the root of your project (default is current directory)
PROJECT_DIR=${1:-$(pwd)}

# Path to the clang-format config file (default is .clang-format in project root)
CLANG_FORMAT_CONFIG=${2:-"$PROJECT_DIR/.clang-format"}

# Check if config file exists
if [[ ! -f "$CLANG_FORMAT_CONFIG" ]]; then
  echo "Error: clang-format config file not found at $CLANG_FORMAT_CONFIG"
  exit 1
fi

# File extensions to format
EXTENSIONS=("*.c" "*.cpp" "*.cc" "*.h" "*.hpp" "*.hh" "*.m" "*.mm")

# Find and apply clang-format to each file
echo "Applying clang-format using config at $CLANG_FORMAT_CONFIG"
for ext in "${EXTENSIONS[@]}"; do
  find "$PROJECT_DIR/src" -type f -name "$ext" -print0 | while IFS= read -r -d '' file; do
    echo "Formatting $file"
    clang-format-14 -i --style=file:"$CLANG_FORMAT_CONFIG" "$file"
  done
  find "$PROJECT_DIR/include" -type f -name "$ext" -print0 | while IFS= read -r -d '' file; do
    echo "Formatting $file"
    clang-format-14 -i --style=file:"$CLANG_FORMAT_CONFIG" "$file"
  done
done

echo "Formatting complete."
