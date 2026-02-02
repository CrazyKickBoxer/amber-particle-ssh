#!/bin/bash
# Restore script for Animation Backup
# Created: 2026-01-22

BACKUP_FILE="antigrav_animation_backup_20260122_071624.tar.gz"
RESTORE_DIR="/home/josh/antigrav_restored_animation"

echo "=== AmberParticleSSH Animation Backup Restore ==="
echo ""

if [ ! -f "$BACKUP_FILE" ]; then
    # Try to find the backup in parent directory
    if [ -f "../$BACKUP_FILE" ]; then
        BACKUP_FILE="../$BACKUP_FILE"
    else
        echo "ERROR: Backup file not found: $BACKUP_FILE"
        echo "Please ensure the backup file is in the current or parent directory."
        exit 1
    fi
fi

echo "Backup file: $BACKUP_FILE"
echo "Restoring to: $RESTORE_DIR"
echo ""

# Create restore directory
mkdir -p "$RESTORE_DIR"

# Extract
echo "Extracting backup..."
tar -xzvf "$BACKUP_FILE" -C "$RESTORE_DIR" --strip-components=0

echo ""
echo "=== Restore Complete ==="
echo ""
echo "Files restored to: $RESTORE_DIR"
echo ""
echo "To build:"
echo "  cd $RESTORE_DIR"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make -j\$(nproc)"
echo ""
echo "To run:"
echo "  ./build/AmberParticleSSH"
