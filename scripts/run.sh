#!/usr/bin/env bash
set -euo pipefail

# run.sh - run the built binary with root privileges if necessary
# Usage: ./run.sh [args...]
# If not root, this script will re-exec the binary via sudo preserving args.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/.."
cd "$PROJECT_ROOT"

BINARY="./build/spoofeye"

if [ ! -x "$BINARY" ]; then
  echo "Binary $BINARY not found or not executable. Build first (e.g. make)." >&2
  exit 1
fi

# If already running as root, exec directly
if [ "$(id -u)" -eq 0 ]; then
  echo "Running as root: $BINARY $*"
  exec "$BINARY" "$@"
fi

# Not root -> try to run with sudo
if command -v sudo >/dev/null 2>&1; then
  echo "Elevating with sudo: $BINARY $*"
  exec sudo --preserve-env=PATH "$BINARY" --config-path "$PROJECT_ROOT/resources/dev.ini" "$@"
else
  echo "This script requires root privileges to capture packets. Please install sudo or run as root." >&2
  exit 1
fi
