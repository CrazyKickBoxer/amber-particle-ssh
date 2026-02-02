#!/bin/bash
# Restore script for Golden Backup V2
# Created: 2026-01-22

# Find the latest V2 backup
BACKUP_FILE=$(ls -t ../antigrav_golden_backup_v2_*.tar.gz | head -n1)
RESTORE_DIR="/home/josh/antigrav_restored_v2"

echo "=== AmberParticleSSH Golden Backup V2 Restore ==="
echo ""

if [ -z "$BACKUP_FILE" ]; then
    echo "ERROR: No 'antigrav_golden_backup_v2_*.tar.gz' found in parent directory."
    exit 1
fi

echo "Found backup: $BACKUP_FILE"
echo "Restoring to: $RESTORE_DIR"
echo ""

# Create restore directory
rm -rf "$RESTORE_DIR"
mkdir -p "$RESTORE_DIR"

# Extract
echo "Extracting..."
tar -xzvf "$BACKUP_FILE" -C "$RESTORE_DIR" --strip-components=0

echo ""
echo "=== Restore Complete ==="
echo ""
echo "To build:"
echo "  cd $RESTORE_DIR"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make -j\$(nproc)"
echo ""
echo "To run:"
echo "  ./build/AmberParticleSSH"
