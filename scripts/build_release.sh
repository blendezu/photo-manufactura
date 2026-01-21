#!/bin/bash
# Photo Manufactura - Complete Release Builder
# Builds distributable packages for the current platform

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Read version from centralized VERSION file
if [ -f "$PROJECT_ROOT/VERSION" ]; then
    VERSION=$(cat "$PROJECT_ROOT/VERSION" | tr -d '[:space:]')
else
    VERSION="0.1.0"
    echo "⚠️  VERSION file not found, using default: $VERSION"
fi

echo "🚀 Photo Manufactura Release Builder v${VERSION}"
echo "================================================"
echo ""

# Detect platform
PLATFORM=$(uname -s)

case $PLATFORM in
    Darwin)
        PLATFORM_NAME="macOS"
        ;;
    Linux)
        PLATFORM_NAME="Linux"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        PLATFORM_NAME="Windows"
        ;;
    *)
        echo "❌ Unsupported platform: $PLATFORM"
        exit 1
        ;;
esac

echo "📍 Detected platform: $PLATFORM_NAME"
echo ""

# Step 1: Generate icon
echo "🎨 Step 1/4: Generating application icon..."
if [ -f "$PROJECT_ROOT/resources/generate_icon.sh" ]; then
    cd "$PROJECT_ROOT/resources"
    ./generate_icon.sh
    cd "$PROJECT_ROOT"
    echo "✅ Icon generated"
else
    echo "⚠️  Icon generation script not found, skipping"
fi
echo ""

# Step 2: Build Release
echo "🔨 Step 2/4: Building Release version..."
if [ -d "$PROJECT_ROOT/build" ]; then
    echo "Cleaning previous build..."
    rm -rf "$PROJECT_ROOT/build"
fi

CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release"
if [ "$PLATFORM_NAME" = "Linux" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=/usr"
fi

cmake -B build -S "$PROJECT_ROOT" -G Ninja $CMAKE_ARGS
cmake --build build --config Release

echo "✅ Build complete"
echo ""

# Step 3: Test build
echo "🧪 Step 3/4: Testing build..."
EXECUTABLE=""
case $PLATFORM_NAME in
    macOS)
        EXECUTABLE="$PROJECT_ROOT/build/bin/photo_manufactura.app/Contents/MacOS/photo_manufactura"
        ;;
    Linux)
        EXECUTABLE="$PROJECT_ROOT/build/bin/photo_manufactura"
        ;;
    Windows)
        EXECUTABLE="$PROJECT_ROOT/build/bin/Release/photo_manufactura.exe"
        ;;
esac

if [ -f "$EXECUTABLE" ] || [ -d "$PROJECT_ROOT/build/bin/photo_manufactura.app" ]; then
    echo "✅ Executable found"
else
    echo "❌ Executable not found at expected location"
    echo "Expected: $EXECUTABLE"
    exit 1
fi
echo ""

# Step 4: Create distribution package
echo "📦 Step 4/4: Creating distribution package..."
case $PLATFORM_NAME in
    macOS)
        if [ -f "$SCRIPT_DIR/create_macos_dmg.sh" ]; then
            bash "$SCRIPT_DIR/create_macos_dmg.sh"
        else
            echo "Creating simple ZIP..."
            cd "$PROJECT_ROOT/build/bin"
            ZIP_NAME="Photo_Manufactura_v${VERSION}_macOS.zip"
            zip -r "$PROJECT_ROOT/$ZIP_NAME" photo_manufactura.app
            echo "✅ Created: $ZIP_NAME"
        fi
        ;;
    Linux)
        if [ -f "$SCRIPT_DIR/create_linux_appimage.sh" ]; then
            bash "$SCRIPT_DIR/create_linux_appimage.sh"
        else
            echo "Creating tarball..."
            cd "$PROJECT_ROOT/build"
            TAR_NAME="Photo_Manufactura_v${VERSION}_Linux.tar.gz"
            tar -czf "$PROJECT_ROOT/$TAR_NAME" bin/
            echo "✅ Created: $TAR_NAME"
        fi
        ;;
    Windows)
        echo "Creating ZIP..."
        cd "$PROJECT_ROOT/build/bin/Release"
        ZIP_NAME="Photo_Manufactura_v${VERSION}_Windows.zip"
        7z a "$PROJECT_ROOT/$ZIP_NAME" * 2>/dev/null || zip -r "$PROJECT_ROOT/$ZIP_NAME" *
        echo "✅ Created: $ZIP_NAME"
        ;;
esac

echo ""
echo "================================================"
echo "✅ Release build complete!"
echo ""
echo "📦 Distribution packages:"
ls -lh "$PROJECT_ROOT"/*.{zip,dmg,AppImage,tar.gz} 2>/dev/null | awk '{print "   " $9 " (" $5 ")"}'
echo ""
echo "📤 Next steps:"
echo "   1. Test the package on a clean system"
echo "   2. Create a GitHub release"
echo "   3. Upload the distribution package"
echo "   4. Share with users!"
echo ""
echo "📚 See docs/DEPLOYMENT.md for distribution options"
