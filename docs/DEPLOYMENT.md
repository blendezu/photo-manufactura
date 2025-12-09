# Photo Manufactura - Deployment & Distribution Guide

## 📦 Overview

This guide covers how to package and distribute Photo Manufactura for end users across different platforms.

---

## 🍎 macOS Deployment

### Option 1: Simple App Bundle (Recommended for Testing)

**Build as standalone .app:**
```bash
# 1. Generate icon
cd resources && ./generate_icon.sh && cd ..

# 2. Build Release version
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 3. The app is ready at:
# build/bin/photo_manufactura.app
```

**Share the app:**
```bash
# Create a distributable zip
cd build/bin
zip -r Photo_Manufactura_v0.1.0.zip photo_manufactura.app
```

Recipients can unzip and drag to `/Applications`.

### Option 2: Signed & Notarized .app (For Public Distribution)

For distribution outside the App Store, you need:
1. Apple Developer account ($99/year)
2. Code signing certificate
3. Notarization with Apple

**Setup codesigning:**
```bash
# Sign the app bundle
codesign --deep --force --verify --verbose \
  --sign "Developer ID Application: Your Name (TEAM_ID)" \
  build/bin/photo_manufactura.app

# Verify signing
codesign --verify --deep --strict --verbose=2 \
  build/bin/photo_manufactura.app
```

**Notarize with Apple:**
```bash
# Create a zip for notarization
ditto -c -k --keepParent \
  build/bin/photo_manufactura.app \
  PhotoManufactura.zip

# Submit for notarization (requires Apple ID)
xcrun notarytool submit PhotoManufactura.zip \
  --apple-id "your@email.com" \
  --team-id "TEAM_ID" \
  --password "app-specific-password" \
  --wait

# Staple the notarization ticket
xcrun stapler staple build/bin/photo_manufactura.app
```

### Option 3: DMG Installer (Professional Distribution)

Create a drag-and-drop DMG installer:

**Install create-dmg:**
```bash
brew install create-dmg
```

**Create DMG:**
```bash
# Build script: scripts/create_macos_dmg.sh
create-dmg \
  --volname "Photo Manufactura" \
  --volicon "resources/icon.icns" \
  --window-pos 200 120 \
  --window-size 800 400 \
  --icon-size 100 \
  --icon "photo_manufactura.app" 175 120 \
  --hide-extension "photo_manufactura.app" \
  --app-drop-link 625 120 \
  --no-internet-enable \
  "Photo_Manufactura_v0.1.0.dmg" \
  "build/bin/"
```

### Option 4: Homebrew Cask (Advanced)

For easy installation via Homebrew:

1. Create a cask formula in homebrew-cask repository
2. Users install with: `brew install --cask photo-manufactura`

---

## 🐧 Linux Deployment

### Option 1: AppImage (Universal Linux Binary)

**Install linuxdeploy:**
```bash
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy*.AppImage
```

**Create AppImage:**
```bash
# Build Release
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
DESTDIR=AppDir cmake --install build

# Bundle dependencies
./linuxdeploy-x86_64.AppImage \
  --appdir AppDir \
  --plugin qt \
  --output appimage \
  --desktop-file PhotoManufactura.desktop \
  --icon-file resources/icon.svg
```

Result: `Photo_Manufactura-x86_64.AppImage` - single executable file

### Option 2: Flatpak (Sandboxed Distribution)

**Create flatpak manifest:** `com.photomanufactura.PhotoManufactura.yml`
```yaml
app-id: com.photomanufactura.PhotoManufactura
runtime: org.kde.Platform
runtime-version: '6.8'
sdk: org.kde.Sdk
command: photo_manufactura
finish-args:
  - --socket=wayland
  - --socket=fallback-x11
  - --share=ipc
  - --device=dri
  - --filesystem=home
modules:
  - name: photo-manufactura
    buildsystem: cmake-ninja
    sources:
      - type: git
        url: https://github.com/yourusername/photo-manufactura.git
```

**Build & install:**
```bash
flatpak-builder build-dir com.photomanufactura.PhotoManufactura.yml
flatpak-builder --install build-dir com.photomanufactura.PhotoManufactura.yml
```

### Option 3: Snap Package

**Create snapcraft.yaml:**
```yaml
name: photo-manufactura
version: '0.1.0'
summary: Professional Photo Processing Application
description: |
  Photo Manufactura is a powerful photo editing and processing tool.
  
grade: stable
confinement: strict
base: core22

apps:
  photo-manufactura:
    command: bin/photo_manufactura
    plugs:
      - desktop
      - home
      - opengl
      - x11

parts:
  photo-manufactura:
    plugin: cmake
    source: .
    build-packages:
      - qtbase6-dev
      - libopencv-dev
      - libraw-dev
```

**Build & publish:**
```bash
snapcraft
sudo snap install photo-manufactura_0.1.0_amd64.snap --dangerous
```

### Option 4: .deb/.rpm Packages

**Using CPack (add to CMakeLists.txt):**
```cmake
# Packaging
set(CPACK_GENERATOR "DEB;RPM;TGZ")
set(CPACK_PACKAGE_NAME "photo-manufactura")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Professional Photo Processing")
set(CPACK_PACKAGE_CONTACT "your@email.com")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base, libopencv, libraw")
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase, opencv, LibRaw")
include(CPack)
```

**Build packages:**
```bash
cmake -B build -S .
cmake --build build
cd build
cpack -G DEB  # Creates .deb
cpack -G RPM  # Creates .rpm
```

---

## 🪟 Windows Deployment

### Option 1: Installer with NSIS/Inno Setup

**Add to CMakeLists.txt:**
```cmake
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Photo Manufactura")
    set(CPACK_NSIS_PACKAGE_NAME "Photo Manufactura")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    include(CPack)
endif()
```

**Build installer:**
```cmd
cmake -B build -S . -G "Visual Studio 17 2022"
cmake --build build --config Release
cd build
cpack -G NSIS
```

### Option 2: WinGet Package

Publish to Windows Package Manager:
```yaml
# manifests/p/PhotoManufactura/PhotoManufactura/0.1.0/PhotoManufactura.PhotoManufactura.yaml
PackageIdentifier: PhotoManufactura.PhotoManufactura
PackageVersion: 0.1.0
PackageLocale: en-US
Publisher: Your Name
PackageName: Photo Manufactura
License: MIT
ShortDescription: Professional Photo Processing Application
Installers:
  - Architecture: x64
    InstallerType: exe
    InstallerUrl: https://github.com/.../Photo_Manufactura_Setup.exe
    InstallerSha256: <sha256>
```

---

## 🔧 Dependency Bundling

### macOS: Bundle Qt & Libraries

**Use macdeployqt:**
```bash
# Qt tool to bundle frameworks
/path/to/Qt/6.8.3/macos/bin/macdeployqt \
  build/bin/photo_manufactura.app \
  -always-overwrite

# Bundle ONNX Runtime
cp -r libs/onnxruntime/lib/*.dylib \
  build/bin/photo_manufactura.app/Contents/Frameworks/

# Fix library paths
install_name_tool -change @rpath/libonnxruntime.dylib \
  @executable_path/../Frameworks/libonnxruntime.dylib \
  build/bin/photo_manufactura.app/Contents/MacOS/photo_manufactura
```

### Linux: Bundle with linuxdeploy

Automatically handled by linuxdeploy when creating AppImage.

### Windows: windeployqt

```cmd
windeployqt.exe build\bin\Release\photo_manufactura.exe
```

---

## 📋 Pre-Deployment Checklist

### ✅ Before Distribution

- [ ] Build in **Release mode** (optimizations enabled)
- [ ] Test on clean system without development tools
- [ ] Bundle all required libraries
- [ ] Include AI_models directory
- [ ] Test all features work in bundled app
- [ ] Create README and LICENSE files
- [ ] Document system requirements
- [ ] Create changelog/release notes

### 🔍 Release Build

```bash
# macOS/Linux
cmake -B build -S . -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local

cmake --build build --config Release

# Windows
cmake -B build -S . -G "Visual Studio 17 2022"
cmake --build build --config Release
```

### 📦 What to Include

**Minimum distribution:**
- Application binary/bundle
- AI_models/ directory
- LICENSE file
- README with system requirements

**Professional distribution:**
- Installer/package
- Documentation (PDF/HTML)
- Example images
- Changelog
- Uninstaller

---

## 🌐 Continuous Deployment

### GitHub Releases (Recommended)

**Create GitHub Actions workflow:** `.github/workflows/release.yml`

```yaml
name: Build Release

on:
  push:
    tags:
      - 'v*'

jobs:
  build-macos:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Install dependencies
        run: |
          brew install qt6 opencv libraw ninja cmake librsvg
          
      - name: Generate icon
        run: cd resources && ./generate_icon.sh
        
      - name: Build
        run: |
          cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
          cmake --build build
          
      - name: Create DMG
        run: |
          brew install create-dmg
          ./scripts/create_macos_dmg.sh
          
      - name: Upload Release Asset
        uses: actions/upload-release-asset@v1
        with:
          upload_url: ${{ github.event.release.upload_url }}
          asset_path: ./Photo_Manufactura_v${{ github.ref_name }}.dmg
          asset_name: Photo_Manufactura_${{ github.ref_name }}_macOS.dmg
          asset_content_type: application/x-apple-diskimage
  
  build-linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y qt6-base-dev libopencv-dev libraw-dev
          
      - name: Build AppImage
        run: |
          cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
          cmake --build build
          # ... AppImage creation steps
          
      - name: Upload AppImage
        uses: actions/upload-release-asset@v1
        # ... upload steps

  build-windows:
    runs-on: windows-latest
    # ... Windows build steps
```

### Docker for Consistent Builds

**Create Dockerfile:**
```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    cmake ninja-build \
    qt6-base-dev \
    libopencv-dev \
    libraw-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

CMD ["/app/build/bin/photo_manufactura"]
```

---

## 📊 Distribution Channels

### Direct Download
- Host DMG/AppImage/EXE on website
- GitHub Releases page
- Cloud storage (Google Drive, Dropbox)

### Package Managers
- **macOS**: Homebrew Cask
- **Linux**: Flathub, Snap Store, AUR
- **Windows**: WinGet, Chocolatey

### App Stores
- **macOS**: Mac App Store (requires review)
- **Linux**: Flathub (open platform)
- **Windows**: Microsoft Store

---

## 🔒 Security Considerations

1. **Code Signing**: Sign binaries to prevent security warnings
2. **Notarization**: Required for macOS Catalina+
3. **Sandboxing**: Consider sandboxed versions for app stores
4. **Update Mechanism**: Implement secure auto-updates
5. **Privacy**: Document data collection (if any)

---

## 📈 Size Optimization

**Reduce bundle size:**
```bash
# Strip debug symbols (Release builds)
strip build/bin/photo_manufactura

# Compress with UPX (optional, may break codesigning)
upx --best build/bin/photo_manufactura

# Remove unused Qt modules
# Only bundle required Qt components
```

---

## 🆘 User Support

**Include in distribution:**
1. System requirements document
2. Installation guide
3. Troubleshooting FAQ
4. Contact/support information
5. Bug report template

---

## 🚀 Quick Deploy Commands

**macOS one-liner:**
```bash
cd resources && ./generate_icon.sh && cd .. && \
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release && \
cmake --build build && \
cd build/bin && zip -r Photo_Manufactura_v0.1.0.zip photo_manufactura.app
```

**Linux AppImage:**
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
cmake --build build && \
./create_appimage.sh
```

---

## 📚 Additional Resources

- [Qt Deployment Guide](https://doc.qt.io/qt-6/deployment.html)
- [macOS Code Signing](https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution)
- [AppImage Documentation](https://docs.appimage.org/)
- [Flatpak Tutorial](https://docs.flatpak.org/)
- [NSIS Documentation](https://nsis.sourceforge.io/Docs/)

---

**Next Steps:**
1. Choose your target platform(s)
2. Set up release build pipeline
3. Test on clean systems
4. Create distribution packages
5. Publish and share!
