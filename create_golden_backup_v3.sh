#!/bin/bash
# create_golden_backup_v3.sh
# Backup of Amber Particle SSH v3.0 (Themes, Optimization, Cursor Inversion, Shimmer)

BACKUP_DIR="backups/golden_v3"
mkdir -p "$BACKUP_DIR"

echo "Creating Golden Backup V3..."

# Source Code
cp -r src "$BACKUP_DIR/"
cp -r shaders "$BACKUP_DIR/"
cp CMakeLists.txt "$BACKUP_DIR/"

# Config
cp -r .vscode "$BACKUP_DIR/" 2>/dev/null

echo "Backup created at $BACKUP_DIR"
