#!/usr/bin/env bash
set -euo pipefail


# Simple build helper. It will call make if available, otherwise compile with g++.
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/.."
BINARY="$PROJECT_ROOT/build/spoofeye"
cd "$PROJECT_ROOT"


if command -v make >/dev/null 2>&1; then
echo "Building with make..."
make
else
echo "make not found"
fi


chmod +x $BINARY || true


echo "Build finished. Binary: $BINARY"
