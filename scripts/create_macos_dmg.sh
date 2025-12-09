#!/bin/bash
# Photo Manufactura - macOS DMG Creator
# Creates a professional installer DMG for macOS

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERSION="0.1.0"

echo "📦 Creating Photo Manufactura DMG installer..."

# Check if app exists
APP_BUNDLE="$PROJECT_ROOT/build/bin/photo_manufactura.app"
if [ ! -d "$APP_BUNDLE" ]; then
    echo "❌ Error: App bundle not found at $APP_BUNDLE"
    echo "Please build the app first:"
    echo "  cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release"
    echo "  cmake --build build"
    exit 1
fi

# Check for icon
if [ ! -f "$PROJECT_ROOT/resources/icon.icns" ]; then
    echo "⚠️  Warning: icon.icns not found, generating..."
    cd "$PROJECT_ROOT/resources"
    ./generate_icon.sh
    cd "$PROJECT_ROOT"
fi

# Ensure proper permissions on app bundle
echo "🔧 Setting app bundle permissions..."
chmod -R u+w "$APP_BUNDLE" 2>/dev/null || true

# Kill any running instances
pkill -f "photo_manufactura.app" 2>/dev/null || true
sleep 1

# Deploy Qt frameworks
echo "🔧 Bundling Qt frameworks..."
QT_PATH=$(brew --prefix qt@6)/bin
if [ -x "$QT_PATH/macdeployqt" ]; then
    # Remove code signatures that might interfere
    echo "   Removing existing signatures..."
    find "$APP_BUNDLE" -type f -name "*.dylib" -exec codesign --remove-signature {} \; 2>/dev/null || true
    find "$APP_BUNDLE/Contents/MacOS" -type f -perm +111 -exec codesign --remove-signature {} \; 2>/dev/null || true
    
    echo "   Running macdeployqt..."
    # Use gtimeout if available (brew install coreutils), otherwise run without timeout
    if command -v gtimeout &> /dev/null; then
        gtimeout 60 "$QT_PATH/macdeployqt" "$APP_BUNDLE" -always-overwrite 2>&1 | grep -v "^ERROR:" | grep -v "^WARNING:" || {
            echo "⚠️  macdeployqt had issues, continuing anyway..."
        }
    else
        "$QT_PATH/macdeployqt" "$APP_BUNDLE" -always-overwrite 2>&1 | grep -v "^ERROR:" | grep -v "^WARNING:" || {
            echo "⚠️  macdeployqt had issues, continuing anyway..."
        }
    fi
else
    echo "⚠️  macdeployqt not found at $QT_PATH"
fi

# Bundle ONNX Runtime if exists
if [ -d "$PROJECT_ROOT/libs/onnxruntime/lib" ]; then
    echo "🔧 Bundling ONNX Runtime..."
    mkdir -p "$APP_BUNDLE/Contents/Frameworks"
    chmod -R u+w "$APP_BUNDLE/Contents/Frameworks" 2>/dev/null || true
    cp -f "$PROJECT_ROOT/libs/onnxruntime/lib"/*.dylib \
        "$APP_BUNDLE/Contents/Frameworks/" 2>/dev/null || true
    
    # Fix permissions on copied dylibs
    chmod u+w "$APP_BUNDLE/Contents/Frameworks"/*.dylib 2>/dev/null || true
fi

# Sign the app bundle with ad-hoc signature
echo "🔐 Signing application..."
codesign --force --deep --sign - "$APP_BUNDLE" 2>/dev/null || {
    echo "⚠️  Ad-hoc signing failed, app may not launch on some systems"
}

# Create DMG staging directory
STAGING_DIR="$PROJECT_ROOT/build/dmg_staging"
rm -rf "$STAGING_DIR"
mkdir -p "$STAGING_DIR"

echo "📋 Preparing DMG contents..."
cp -R "$APP_BUNDLE" "$STAGING_DIR/"

# Note: Applications symlink will be created by create-dmg with --app-drop-link

# Add README
cat > "$STAGING_DIR/README.txt" << EOF
Photo Manufactura v${VERSION}

INSTALLATION:
1. Drag "photo_manufactura.app" to the Applications folder
2. Launch from Applications or Spotlight

SYSTEM REQUIREMENTS:
- macOS 11.0 (Big Sur) or later
- Apple Silicon (M1/M2/M3) or Intel processor

FIRST LAUNCH:
If you see a security warning:
1. Right-click the app and select "Open"
2. Click "Open" in the dialog

SUPPORT:
Report issues at: https://github.com/ad-tran/photo-manufactura/issues

Copyright © 2025
EOF

# Check for create-dmg
if ! command -v create-dmg &> /dev/null; then
    echo "⚠️  create-dmg not found, installing..."
    brew install create-dmg
fi

# Create DMG
DMG_NAME="Photo_Manufactura_v${VERSION}_macOS.dmg"
DMG_PATH="$PROJECT_ROOT/$DMG_NAME"

echo "💿 Creating DMG: $DMG_NAME"

create-dmg \
    --volname "Photo Manufactura" \
    --volicon "$PROJECT_ROOT/resources/icon.icns" \
    --window-pos 200 120 \
    --window-size 800 450 \
    --icon-size 100 \
    --icon "photo_manufactura.app" 200 150 \
    --hide-extension "photo_manufactura.app" \
    --app-drop-link 600 150 \
    --background "$PROJECT_ROOT/resources/dmg_background.png" \
    --no-internet-enable \
    "$DMG_PATH" \
    "$STAGING_DIR" 2>/dev/null || {
        # Fallback without background if image doesn't exist
        create-dmg \
            --volname "Photo Manufactura" \
            --volicon "$PROJECT_ROOT/resources/icon.icns" \
            --window-pos 200 120 \
            --window-size 800 450 \
            --icon-size 100 \
            --icon "photo_manufactura.app" 200 150 \
            --hide-extension "photo_manufactura.app" \
            --app-drop-link 600 150 \
            --no-internet-enable \
            "$DMG_PATH" \
            "$STAGING_DIR"
    }

# Clean up
rm -rf "$STAGING_DIR"

# Calculate size
DMG_SIZE=$(du -h "$DMG_PATH" | cut -f1)

echo ""
echo "✅ DMG created successfully!"
echo "📦 File: $DMG_PATH"
echo "💾 Size: $DMG_SIZE"
echo ""
echo "🚀 To distribute:"
echo "   1. Upload to GitHub Releases"
echo "   2. Share download link"
echo "   3. Users drag app to Applications folder"
echo ""
echo "🔒 For public distribution, consider:"
echo "   - Code signing with Developer ID"
echo "   - Notarizing with Apple"
echo "   See docs/DEPLOYMENT.md for details"
