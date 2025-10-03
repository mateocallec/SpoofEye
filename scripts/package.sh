#!/usr/bin/env bash
set -euo pipefail

# -----------------------------
# Paths
# -----------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/.."
DEBIAN_DIR="$PROJECT_ROOT/debian"
PACKAGE_DIR="$PROJECT_ROOT/package"
OUTPUT_DIR="$PROJECT_ROOT/dist"

cd "$PROJECT_ROOT"

# -----------------------------
# Clean previous Debian build & distribution
# -----------------------------
rm -rf "$DEBIAN_DIR"
rm -rf "$OUTPUT_DIR"

# -----------------------------
# Prepare Debian package files
# -----------------------------
mkdir -p "$DEBIAN_DIR"

cp "$PACKAGE_DIR/control"       "$DEBIAN_DIR/"
cp "$PACKAGE_DIR/rules"         "$DEBIAN_DIR/"
cp "$PACKAGE_DIR/install"       "$DEBIAN_DIR/"
cp "$PACKAGE_DIR/changelog"     "$DEBIAN_DIR/"
cp "$PACKAGE_DIR/README.Debian" "$DEBIAN_DIR/"
cp "$PACKAGE_DIR/copyright"     "$DEBIAN_DIR/"
cp "$PACKAGE_DIR/postinst"      "$DEBIAN_DIR/"
cp "$PACKAGE_DIR/postrm"        "$DEBIAN_DIR/"

# -----------------------------
# Build the Debian package
# -----------------------------
dpkg-buildpackage -us -uc

# -----------------------------
# Collect all generated files into the distribution folder
# -----------------------------
mkdir -p "$OUTPUT_DIR"

# Move .deb, .changes, and .buildinfo files from the parent directory (default dpkg-buildpackage output)
mv "$PROJECT_ROOT/../"*.deb      "$OUTPUT_DIR/" || true
mv "$PROJECT_ROOT/../"*.changes "$OUTPUT_DIR/" || true
mv "$PROJECT_ROOT/../"*.buildinfo "$OUTPUT_DIR/" || true
mv "$PROJECT_ROOT/../"*.tar.gz "$OUTPUT_DIR/" || true
mv "$PROJECT_ROOT/../"*.tar.xz "$OUTPUT_DIR/" || true
mv "$PROJECT_ROOT/../"*.dsc "$OUTPUT_DIR/" || true

echo "Debian package build complete. Files are in $OUTPUT_DIR"
