#!/bin/bash
# restore_golden_v3.sh
# Restores Amber Particle SSH v3.0

BACKUP_DIR="backups/golden_v3"

if [ ! -d "$BACKUP_DIR" ]; then
    echo "Error: Backup directory $BACKUP_DIR not found!"
    exit 1
fi

echo "Restoring Golden Backup V3..."

rm -rf src shaders CMakeLists.txt
cp -r "$BACKUP_DIR/src" .
cp -r "$BACKUP_DIR/shaders" .
cp "$BACKUP_DIR/CMakeLists.txt" .

echo "Restore complete. Please rebuild."
