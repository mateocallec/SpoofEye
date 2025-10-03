#!/usr/bin/env bash
# setup.sh - Prepare a development environment for SpoofEye
# - installs system packages required by the project and build system
# - creates a Python virtual environment and installs Python dependencies
#
# Usage: ./setup.sh
# Note: This script uses sudo for system package installation.

set -euo pipefail

# Configuration
PYTHON_BIN="python3"
VENV_DIR="venv"         # venv directory created in project root
REQ_FILE="requirements.txt"

echo "[INFO] Starting setup for SpoofEye development environment."

echo "[INFO] Updating package lists..."
sudo apt update

echo "[INFO] Installing essential build tools..."
sudo apt install -y build-essential g++ make cmake git pkg-config

echo "[INFO] Installing libnotify and glib development libraries..."
sudo apt install -y libnotify-dev libglib2.0-dev

echo "[INFO] Installing networking libraries (libpcap) and headers..."
sudo apt install -y libpcap-dev

echo "[INFO] Installing Python dev packages and venv support..."
sudo apt install -y ${PYTHON_BIN} ${PYTHON_BIN}-dev ${PYTHON_BIN}-venv python3-pip

echo "[INFO] Installing additional useful packages..."
sudo apt install -y wget curl unzip

echo "[INFO] Installing development and debugging tools..."
sudo apt install -y gdb valgrind strace ltrace

echo "[INFO] Installing static analysis and code quality tools..."
sudo apt install -y cppcheck clang-tidy clang-format

echo "[INFO] Installing documentation tools (Doxygen + Graphviz)..."
sudo apt install -y doxygen graphviz

echo "[INFO] Installing packaging tools for Debian builds..."
sudo apt install -y devscripts debhelper dh-make lintian

echo "[INFO] Installing common C++ libraries (nlohmann json)..."
sudo apt install -y nlohmann-json3-dev

# At this point, system packages are installed. Now set up Python venv.
if [ ! -x "$(command -v ${PYTHON_BIN})" ]; then
  echo "[ERROR] ${PYTHON_BIN} is not installed or not found in PATH."
  exit 1
fi

echo "[INFO] Creating Python virtual environment in ./${VENV_DIR} (if not exists)..."
if [ -d "${VENV_DIR}" ]; then
  echo "[INFO] Virtual environment directory '${VENV_DIR}' already exists. Skipping creation."
else
  ${PYTHON_BIN} -m venv "${VENV_DIR}"
  echo "[INFO] Virtual environment created at ./${VENV_DIR}"
fi

# Activate venv for the remainder of this script
# shellcheck disable=SC1090
source "${VENV_DIR}/bin/activate"

echo "[INFO] Upgrading pip, setuptools and wheel inside venv..."
pip install --upgrade pip setuptools wheel

echo "[INFO] Preparing Python package list for SpoofEye..."
# The user-specified imports:
#   socket, uuid, psutil, platform, subprocess, re, sys, scapy, typing
# Most are stdlib; we need to install scapy and psutil and typing related helpers.

PY_DEPS=(
  scapy         # packet manipulation and sniffing
  psutil        # process / system utilities
  typing-extensions  # helpful for backporting typing features on older Python
)

echo "[INFO] Installing Python dependencies into the virtualenv..."
pip install "${PY_DEPS[@]}"

echo "[INFO] Freezing installed Python packages to ${REQ_FILE}..."
pip freeze > "${REQ_FILE}"
echo "[INFO] Wrote ${REQ_FILE} with the current venv packages."

# Post-install checks: verify scapy import works
echo "[INFO] Verifying Python imports (quick check)..."
python - <<'PYCHK'
import sys
try:
    import scapy.all as scapy
    import psutil
    import socket, uuid, platform, subprocess, re, sys, typing
except Exception as e:
    print("[ERROR] One or more Python imports failed:", e)
    sys.exit(2)
else:
    print("[INFO] Python imports OK (scapy, psutil, stdlib modules).")
PYCHK

# Provide helpful developer tips
deactivate || true

echo "--------------------------------------------------"
echo "[INFO] Setup complete!"
echo ""
echo "Python virtual environment created at: ./${VENV_DIR}"
echo "To start developing, activate it with:"
echo ""
echo "  source ${VENV_DIR}/bin/activate"
echo ""
echo "Then run your Python scripts (example):"
echo ""
echo "  source ${VENV_DIR}/bin/activate && python -c \"from scapy.all import *; print('scapy OK')\""
echo ""
echo "Requirements have been recorded to: ${REQ_FILE}"
echo ""
echo "System packages installed include: build-essential, libpcap-dev, libnotify-dev, python3-venv, doxygen, lintian, and more."
echo ""
echo "[TIP] If you are going to use network capture features, run the binary or scripts with sufficient privileges"
echo "      (e.g., using capabilities: 'sudo setcap cap_net_raw,cap_net_admin=eip ./build/spoofeye' is preferred to running as root)."
echo ""
echo "You can now build the C++ project using 'make' or './build.sh' if you create a wrapper."
echo "--------------------------------------------------"
