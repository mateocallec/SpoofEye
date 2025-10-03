#!/usr/bin/env bash
set -euo pipefail

# Simple build helper. It will call make if available, otherwise compile with g++.
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/.."
OUTPUTS="$PROJECT_ROOT/outputs"
cd "$PROJECT_ROOT"


if command -v make >/dev/null 2>&1; then
echo "Building with make..."
make clean
rm -rf $OUTPUTS
else
echo "make not found"
fi

# Clean Debian package
rm -rf $PROJECT_ROOT/debian
rm -rf $PROJECT_ROOT/.spoofeye

echo "Project cleaned."
