# 📦 Deployment & Shipping - Complete Setup

## ✅ What Was Created

Your Photo Manufactura project now has a complete deployment system!

### 🛠️ Deployment Scripts
```
scripts/
├── build_release.sh           # 🚀 One-command release builder
├── create_macos_dmg.sh        # 💿 Professional DMG installer
└── create_linux_appimage.sh   # 🐧 Universal Linux binary
```

### 📚 Documentation
```
docs/
├── DEPLOYMENT.md              # 📖 Complete deployment guide (all platforms)
├── DEPLOYMENT_QUICK.md        # ⚡ Quick start (5 min setup)
├── SYSTEM_REQUIREMENTS.md     # 💻 User system requirements
└── LAUNCHER.md               # 🎯 Launcher setup guide

Root files:
├── DEPLOYMENT_README.md       # 📋 Deployment summary (this file)
└── LAUNCHER_QUICKSTART.md    # 🚀 Quick launcher guide
```

### 🤖 Automation
```
.github/workflows/
└── release.yml               # ⚙️ Auto-build on GitHub (macOS, Linux, Windows)
```

### 🎨 Assets
```
resources/
├── icon.svg                  # 🎨 Application icon (camera design)
└── generate_icon.sh          # 🔧 Icon converter script
```

---

## 🎯 Quick Start - Ship Your App

### For macOS (Your Platform)

**Option A: Automated (Recommended)**
```bash
./scripts/build_release.sh
```

**Option B: Manual Steps**
```bash
# 1. Generate icon (one-time)
cd resources && ./generate_icon.sh && cd ..

# 2. Build release
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 3. Create DMG
./scripts/create_macos_dmg.sh
```

**Result**: `Photo_Manufactura_v0.1.0_macOS.dmg` ready to distribute!

---

## 📤 Distribution Methods

### 1️⃣ GitHub Releases (Easiest)
```bash
# Install GitHub CLI (one-time)
brew install gh

# Create release
gh release create v0.1.0 Photo_Manufactura*.dmg \
  --title "Photo Manufactura v0.1.0" \
  --notes "Initial release"
```

✅ Users download from: `github.com/yourname/photo-manufactura/releases`

### 2️⃣ Direct Download
Upload DMG to:
- Your website
- Google Drive / Dropbox (share public link)
- WeTransfer
- AWS S3 / DigitalOcean Spaces

### 3️⃣ Automated (GitHub Actions)
Push a tag:
```bash
git tag v0.1.0
git push origin v0.1.0
```

GitHub automatically builds and publishes releases for:
- ✅ macOS
- ✅ Linux
- ✅ Windows

---

## 📊 Package Contents

Your distribution is **self-contained** and includes:

| Component | What Users Get |
|-----------|----------------|
| 📱 Application | Photo Manufactura.app |
| 🎨 Icon | Professional camera icon |
| 📦 Qt Frameworks | All UI libraries bundled |
| 🖼️ OpenCV | Image processing bundled |
| 📷 LibRaw | RAW file support bundled |
| 🤖 AI Models | ~300 MB of neural networks |
| 📄 Documentation | README and license |

**Total Size**: ~500 MB
**User Action**: Just drag to Applications folder!

---

## 🎓 Documentation Guide

### For You (Deploying the App)

**Start here** → `docs/DEPLOYMENT_QUICK.md` (5-minute guide)

Then explore:
- `docs/DEPLOYMENT.md` - Complete guide for all platforms
- `.github/workflows/release.yml` - Automated builds
- `scripts/build_release.sh` - Build automation

### For Your Users

**Getting started** → `LAUNCHER_QUICKSTART.md`

Additional info:
- `docs/SYSTEM_REQUIREMENTS.md` - What they need
- `docs/LAUNCHER.md` - Detailed launcher setup
- `README.md` - How to use the app

---

## ✅ Pre-Release Checklist

Before sharing with users:

```bash
# ✓ Icon generated?
ls resources/icon.icns

# ✓ Release build (not debug)?
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build

# ✓ Test the packaged app
./launch_photo_manufactura.sh

# ✓ AI models included?
ls build/bin/AI_models/

# ✓ Create distribution package
./scripts/build_release.sh

# ✓ Test on clean system (or VM)
# Download and test as if you're a user

# ✓ Create release on GitHub
gh release create v0.1.0 Photo_Manufactura*.dmg
```

---

## 🔐 Security Notes

### Gatekeeper Warning (macOS)

Users might see: **"Can't open because it is from an unidentified developer"**

**Quick fix for users:**
- Right-click app → Open → Click "Open"

OR:
```bash
xattr -cr /Applications/photo_manufactura.app
```

**Permanent solution** (requires Apple Developer account):
- Code sign with Developer ID
- Notarize with Apple
- See `docs/DEPLOYMENT.md` section on "Signed & Notarized"

---

## 🚀 Platform-Specific Deployment

### macOS (Your Platform) ✅
```bash
# Quick deploy
./scripts/build_release.sh

# Or DMG only
./scripts/create_macos_dmg.sh
```

**Result**: Professional DMG installer

### Linux 🐧
```bash
# AppImage (universal binary)
./scripts/create_linux_appimage.sh
```

**Result**: Single executable file that runs anywhere

### Windows 🪟
Use GitHub Actions or:
```cmd
cmake -B build -S . -G "Visual Studio 17 2022"
cmake --build build --config Release
windeployqt build\bin\Release\photo_manufactura.exe
```

**Result**: Folder with .exe and dependencies

---

## 🤖 Continuous Deployment

### Automated Releases (Already Set Up!)

Every time you push a tag:
```bash
git tag v0.2.0
git push origin v0.2.0
```

GitHub Actions automatically:
1. ✅ Builds for macOS, Linux, Windows
2. ✅ Creates distribution packages
3. ✅ Publishes to GitHub Releases
4. ✅ Users can download immediately

See: `.github/workflows/release.yml`

---

## 💡 Version Management

Update version in the `VERSION` file at project root:
```bash
cat VERSION  # View current version
echo "0.2.0" > VERSION  # Update version
```

The CMake build system and release scripts automatically read from this file.

### Versioning Scheme
- `v0.1.0` - Initial beta
- `v1.0.0` - First stable release
- `v1.1.0` - New features
- `v1.1.1` - Bug fixes

---

## 🐛 Troubleshooting

### "Build failed"
```bash
rm -rf build
./scripts/build_release.sh
```

### "Icon not showing"
```bash
cd resources && ./generate_icon.sh && cd ..
cmake --build build
```

### "Library not loaded"
```bash
# Bundle Qt
macdeployqt build/bin/photo_manufactura.app
```

### "File too large"
Normal! Qt apps are ~500 MB. Users prefer "just works" over small size.

---

## 📈 Release Workflow

For each new version:

1. **Update version** in CMakeLists.txt
2. **Test locally**
3. **Commit changes**
4. **Create tag**: `git tag v0.2.0`
5. **Push tag**: `git push origin v0.2.0`
6. **GitHub Actions** builds automatically
7. **Announce** on social media

---

## 📞 User Support

If users report issues:

1. Check `docs/SYSTEM_REQUIREMENTS.md`
2. Ask them to use launcher: `./launch_photo_manufactura.sh`
3. Check macOS version (needs 11.0+)
4. Remove quarantine: `xattr -cr photo_manufactura.app`

---

## 🎉 You're Ready!

### Everything is set up for deployment:

✅ Build scripts ready
✅ Documentation complete  
✅ Icon and assets prepared
✅ GitHub Actions configured
✅ User guides written

### Next Steps:

1. Run `./scripts/build_release.sh`
2. Test the package
3. Create GitHub release
4. Share with the world! 🌍

---

## 🚢 Deploy Now!

**One command to rule them all:**
```bash
./scripts/build_release.sh && echo "🎉 Ready to ship!"
```

**Happy shipping! 🚀**

---

## 📚 Quick Reference

| Task | Command |
|------|---------|
| Build release | `./scripts/build_release.sh` |
| Create DMG | `./scripts/create_macos_dmg.sh` |
| Test app | `./launch_photo_manufactura.sh` |
| GitHub release | `gh release create v0.1.0 *.dmg` |
| Auto-deploy | `git push origin v0.1.0` |

| Document | Purpose |
|----------|---------|
| `DEPLOYMENT_QUICK.md` | Quick start (5 min) |
| `DEPLOYMENT.md` | Complete guide |
| `SYSTEM_REQUIREMENTS.md` | For users |
| `LAUNCHER.md` | Launcher setup |
