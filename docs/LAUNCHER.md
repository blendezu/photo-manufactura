# Photo Manufactura Launcher Setup

This guide explains how to set up a desktop launcher for Photo Manufactura on macOS.

## Quick Start

### Option 1: Simple Launch Script (Recommended)

The easiest way to launch Photo Manufactura is using the provided launch script:

```bash
./launch_photo_manufactura.sh
```

This script:
- Automatically finds the built executable
- Ensures AI models are in place
- Provides helpful error messages if the build is missing

### Option 2: macOS App Bundle (Native)

Photo Manufactura can be built as a native macOS application bundle with an icon.

#### Prerequisites

1. **Generate the icon file** (one-time setup):
   ```bash
   # Install librsvg if not already installed
   brew install librsvg
   
   # Generate the .icns file from the SVG icon
   cd resources
   ./generate_icon.sh
   ```

2. **Build the application**:
   ```bash
   # Configure with CMake
   cmake -B build -S . -G Ninja
   
   # Build the project
   cmake --build build
   ```

   This will create `Photo Manufactura.app` in the `build/bin/` directory.

3. **Launch the app**:
   - Double-click `build/bin/photo_manufactura.app` in Finder
   - Or from terminal: `open build/bin/photo_manufactura.app`

#### Create a Desktop Shortcut

To add Photo Manufactura to your desktop or Applications folder:

**Method 1: Copy to Applications (Recommended)**
```bash
# Copy the app bundle to Applications folder
cp -r build/bin/photo_manufactura.app /Applications/

# Launch from Applications
open /Applications/photo_manufactura.app
```

**Method 2: Create an Alias on Desktop**
1. Open Finder and navigate to `build/bin/`
2. Right-click on `photo_manufactura.app`
3. Select "Make Alias"
4. Drag the alias to your Desktop or Applications folder

**Method 3: Add to Dock**
1. Open the app from `build/bin/photo_manufactura.app`
2. Right-click the app icon in the Dock
3. Select "Options" → "Keep in Dock"

## Files Overview

### Icon Files
- **`resources/icon.svg`** - Vector icon source file (512x512)
- **`resources/icon.icns`** - macOS icon file (generated from SVG)
- **`resources/generate_icon.sh`** - Script to generate .icns from SVG

### Launcher Files
- **`launch_photo_manufactura.sh`** - Simple bash script to launch the app
- **`PhotoManufactura.desktop`** - Desktop entry file (for Linux compatibility)

### Application Structure
When built as a macOS bundle, the structure is:
```
photo_manufactura.app/
├── Contents/
│   ├── Info.plist
│   ├── MacOS/
│   │   └── photo_manufactura (executable)
│   └── Resources/
│       ├── icon.icns
│       └── AI_models/ (automatically copied)
```

## Customizing the Icon

If you want to create a custom icon:

1. **Edit the SVG**: Open `resources/icon.svg` in any vector graphics editor
2. **Regenerate the .icns**:
   ```bash
   cd resources
   ./generate_icon.sh
   ```
3. **Rebuild the app**:
   ```bash
   cmake --build build
   ```

## Troubleshooting

### "Cannot open app because it is from an unidentified developer"

This is a macOS Gatekeeper warning. To bypass it:
```bash
xattr -cr build/bin/photo_manufactura.app
```

Or, right-click the app and select "Open", then click "Open" in the dialog.

### Icon not showing

1. Verify the icon file exists:
   ```bash
   ls -l resources/icon.icns
   ```

2. Regenerate if needed:
   ```bash
   cd resources
   ./generate_icon.sh
   ```

3. Clear the icon cache:
   ```bash
   sudo rm -rf /Library/Caches/com.apple.iconservices.store
   killall Finder
   ```

4. Rebuild the application:
   ```bash
   cmake --build build
   ```

### AI Models not found

The launcher script automatically copies AI models from the project root to the build directory. If you see errors:

1. Ensure `AI_models/` directory exists in project root
2. Rebuild or run the launch script which handles copying

## Building from Scratch

Complete build process:
```bash
# 1. Generate icon (one time)
cd resources
./generate_icon.sh
cd ..

# 2. Configure CMake
cmake -B build -S . -G Ninja

# 3. Build
cmake --build build

# 4. Launch
./launch_photo_manufactura.sh
# or
open build/bin/photo_manufactura.app
```

## Installing for All Users

To make Photo Manufactura available system-wide:

```bash
# Install to /Applications
sudo cmake --install build

# Or manually copy
sudo cp -r build/bin/photo_manufactura.app /Applications/
```

## Uninstalling

To remove Photo Manufactura:

```bash
# Remove from Applications
rm -rf /Applications/photo_manufactura.app

# Remove from Dock (if pinned)
# Right-click icon → Options → Remove from Dock
```
