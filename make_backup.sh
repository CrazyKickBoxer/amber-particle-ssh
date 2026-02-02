#!/bin/bash
set -e

BACKUP_DIR="backups/golden_2026_01_20"

echo "Creating backup in $BACKUP_DIR..."
mkdir -p "$BACKUP_DIR"

if [ -d "src" ]; then
    cp -r src "$BACKUP_DIR/"
else
    echo "Error: src directory not found!"
    exit 1
fi

if [ -d "shaders" ]; then
    cp -r shaders "$BACKUP_DIR/"
else
    echo "Error: shaders directory not found!"
    exit 1
fi

if [ -f "CMakeLists.txt" ]; then
    cp CMakeLists.txt "$BACKUP_DIR/"
else
    echo "Error: CMakeLists.txt not found!"
    exit 1
fi

# Create internal restore script (optional, but good to have self-contained backup)
cat > "$BACKUP_DIR/restore_here.sh" << 'EOF'
#!/bin/bash
echo "Restoring to ../../..."
cp -r src ../../
cp -r shaders ../../
cp CMakeLists.txt ../../
echo "Done."
EOF
chmod +x "$BACKUP_DIR/restore_here.sh"

echo "Backup created successfully at $BACKUP_DIR"
