#!/bin/bash
# Photo Manufactura - Linux AppImage Creator
# Creates a universal Linux binary using AppImage

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERSION="0.1.0"

echo "📦 Creating Photo Manufactura AppImage..."

# Check if build exists
BUILD_DIR="$PROJECT_ROOT/build"
if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Error: Build directory not found"
    echo "Please build the project first:"
    echo "  cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr"
    echo "  cmake --build build"
    exit 1
fi

# Install to AppDir
APPDIR="$PROJECT_ROOT/AppDir"
rm -rf "$APPDIR"

echo "📋 Installing to AppDir..."
DESTDIR="$APPDIR" cmake --install "$BUILD_DIR"

# Create AppDir structure
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/scalable/apps"
mkdir -p "$APPDIR/usr/share/metainfo"

# Copy desktop file
cp "$PROJECT_ROOT/PhotoManufactura.desktop" \
   "$APPDIR/usr/share/applications/com.photomanufactura.PhotoManufactura.desktop"

# Copy icon
if [ -f "$PROJECT_ROOT/resources/icon.svg" ]; then
    cp "$PROJECT_ROOT/resources/icon.svg" \
       "$APPDIR/usr/share/icons/hicolor/scalable/apps/photo_manufactura.svg"
fi

# Create AppRun script
cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash
APPDIR="$(dirname "$(readlink -f "$0")")"
export LD_LIBRARY_PATH="$APPDIR/usr/lib:$LD_LIBRARY_PATH"
export QT_PLUGIN_PATH="$APPDIR/usr/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="$APPDIR/usr/plugins/platforms"

# Run the application
exec "$APPDIR/usr/bin/photo_manufactura" "$@"
EOF
chmod +x "$APPDIR/AppRun"

# Create metainfo
cat > "$APPDIR/usr/share/metainfo/com.photomanufactura.PhotoManufactura.appdata.xml" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>com.photomanufactura.PhotoManufactura</id>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>MIT</project_license>
  <name>Photo Manufactura</name>
  <summary>Professional Photo Processing Application</summary>
  <description>
    <p>
      Photo Manufactura is a powerful photo editing and processing tool
      with AI-powered features including denoising, style transfer, and
      advanced RAW processing capabilities.
    </p>
  </description>
  <launchable type="desktop-id">com.photomanufactura.PhotoManufactura.desktop</launchable>
  <releases>
    <release version="${VERSION}" date="$(date +%Y-%m-%d)"/>
  </releases>
</component>
EOF

# Download linuxdeploy if not present
LINUXDEPLOY="$PROJECT_ROOT/scripts/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT="$PROJECT_ROOT/scripts/linuxdeploy-plugin-qt-x86_64.AppImage"

if [ ! -f "$LINUXDEPLOY" ]; then
    echo "📥 Downloading linuxdeploy..."
    wget -O "$LINUXDEPLOY" \
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    chmod +x "$LINUXDEPLOY"
fi

if [ ! -f "$LINUXDEPLOY_QT" ]; then
    echo "📥 Downloading linuxdeploy-plugin-qt..."
    wget -O "$LINUXDEPLOY_QT" \
        "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
    chmod +x "$LINUXDEPLOY_QT"
fi

# Create AppImage
echo "🔧 Bundling dependencies..."
cd "$PROJECT_ROOT"

OUTPUT_NAME="Photo_Manufactura-${VERSION}-x86_64.AppImage"

"$LINUXDEPLOY" \
    --appdir "$APPDIR" \
    --plugin qt \
    --output appimage \
    --desktop-file "$APPDIR/usr/share/applications/com.photomanufactura.PhotoManufactura.desktop" \
    --icon-file "$PROJECT_ROOT/resources/icon.svg"

# Rename output
if [ -f "Photo_Manufactura-${VERSION}-x86_64.AppImage" ]; then
    mv "Photo_Manufactura-x86_64.AppImage" "$OUTPUT_NAME" 2>/dev/null || true
fi

# Make executable
chmod +x "$OUTPUT_NAME"

APPIMAGE_SIZE=$(du -h "$OUTPUT_NAME" | cut -f1)

echo ""
echo "✅ AppImage created successfully!"
echo "📦 File: $OUTPUT_NAME"
echo "💾 Size: $APPIMAGE_SIZE"
echo ""
echo "🚀 To use:"
echo "   chmod +x $OUTPUT_NAME"
echo "   ./$OUTPUT_NAME"
echo ""
echo "📤 To distribute:"
echo "   - Upload to GitHub Releases"
echo "   - Publish to Flathub"
echo "   - Share direct download link"
