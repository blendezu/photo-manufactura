# Photo Manufactura - Quick Launch Guide

## 🚀 Quick Start

### Launch the application right now:
```bash
./launch_photo_manufactura.sh
```

## 📱 Create Desktop Shortcut

### For macOS:

1. **Generate the icon** (one-time):
   ```bash
   brew install librsvg
   cd resources && ./generate_icon.sh && cd ..
   ```

2. **Build the app bundle**:
   ```bash
   cmake -B build -S . -G Ninja
   cmake --build build
   ```

3. **Copy to Applications**:
   ```bash
   cp -r build/bin/photo_manufactura.app /Applications/
   ```

4. **Launch**: Double-click `Photo Manufactura` in your Applications folder!

## 📖 Full Documentation

See `docs/LAUNCHER.md` for complete instructions including:
- Desktop shortcuts
- Dock integration
- Custom icons
- Troubleshooting

---

**Files created:**
- 🎨 `resources/icon.svg` - Application icon (camera design)
- 🔧 `resources/generate_icon.sh` - Icon converter script
- 🚀 `launch_photo_manufactura.sh` - Simple launcher script
- 📋 `PhotoManufactura.desktop` - Desktop entry file
- 📚 `docs/LAUNCHER.md` - Complete setup guide
