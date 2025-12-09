# 🚀 Photo Manufactura - Quick Deployment Guide

## For Developers: Ship Your App

### 🎯 Quick Deploy (5 minutes)

**macOS users:**
```bash
# One command to build and package
./scripts/build_release.sh
```

This creates: `Photo_Manufactura_v0.1.0_macOS.dmg` or `.zip`

**Share with users:**
1. Upload to GitHub Releases
2. Send download link
3. Users drag to Applications folder ✅

---

## 📦 What Gets Packaged

Your distribution includes:
- ✅ Application binary
- ✅ All required libraries (Qt, OpenCV, etc.)
- ✅ AI models (~300 MB)
- ✅ Application icon
- ✅ README and documentation

---

## 🎨 Distribution Options

### Option 1: Simple ZIP (Fastest)
```bash
cd build/bin
zip -r Photo_Manufactura.zip photo_manufactura.app
```
**Pros**: Fast, simple
**Cons**: No installer, manual to Applications folder

### Option 2: DMG Installer (Professional)
```bash
./scripts/create_macos_dmg.sh
```
**Pros**: Professional, drag-and-drop installer, auto-opens
**Cons**: Requires `create-dmg` tool

### Option 3: Signed & Notarized (Public Release)
Requires Apple Developer account ($99/year)
See: `docs/DEPLOYMENT.md` section "Signed & Notarized"

---

## 🌐 Where to Host

### GitHub Releases (Recommended)
```bash
# Create a release on GitHub
gh release create v0.1.0 \
  Photo_Manufactura_v0.1.0_macOS.dmg \
  --title "Photo Manufactura v0.1.0" \
  --notes "Initial release"
```

Users download from: `https://github.com/yourusername/photo-manufactura/releases`

### Other Options
- Your website
- Google Drive / Dropbox (make public)
- WeTransfer (temporary links)
- Cloud storage (AWS S3, DigitalOcean Spaces)

---

## ✅ Pre-Ship Checklist

Before distributing, verify:

- [ ] Built in **Release mode** (`-DCMAKE_BUILD_TYPE=Release`)
- [ ] Tested on clean Mac (without dev tools installed)
- [ ] Icon shows correctly
- [ ] AI models load properly
- [ ] All features work in bundled app
- [ ] Created README.txt or documentation
- [ ] Added LICENSE file
- [ ] Wrote release notes / changelog

---

## 🧪 Test Your Package

**On a clean system (or ask a friend):**
1. Download your DMG/ZIP
2. Install/extract the app
3. Launch without development tools
4. Test all features
5. Check for missing dependencies

**Virtual machine testing:**
- Use Parallels/VMware
- Install clean macOS
- Test your package

---

## 📊 Package Size Tips

**Current size**: ~500 MB total

**To reduce size:**
- Remove unused AI models
- Strip debug symbols: `strip build/bin/photo_manufactura`
- Use Qt minimal plugins only
- Compress with UPX (careful with codesigning)

**To keep size reasonable:**
- Don't bundle example images in app
- Provide AI models as optional download
- Use delta updates for newer versions

---

## 🔐 Security & Trust

### For macOS Gatekeeper

**Users see warning: "Cannot open unverified app"**

**Solution 1** (free): Tell users to right-click → Open

**Solution 2** (paid): Get Apple Developer ID
```bash
# Sign your app
codesign --deep --force --sign "Developer ID" \
  photo_manufactura.app

# Notarize with Apple
xcrun notarytool submit Photo_Manufactura.dmg \
  --apple-id "your@email.com" --wait
```

---

## 📈 Distribution Channels

### Immediate (Free)
1. **GitHub Releases** - Easy, free, built-in
2. **Direct download** - Your website
3. **Social media** - Twitter, Reddit, forums

### Later (Optional)
1. **Homebrew Cask** - `brew install photo-manufactura`
2. **Mac App Store** - Requires review
3. **Setapp** - Subscription platform

---

## 🚀 Launch Workflow

**For each release:**

1. **Build**
   ```bash
   ./scripts/build_release.sh
   ```

2. **Test** on clean system

3. **Create release notes**
   ```
   v0.1.0 - 2025-12-09
   - Initial release
   - AI-powered denoising
   - Style transfer
   - RAW processing
   ```

4. **Upload to GitHub**
   ```bash
   gh release create v0.1.0 Photo_Manufactura_*.dmg \
     --notes-file CHANGELOG.md
   ```

5. **Announce**
   - Blog post
   - Social media
   - Product Hunt
   - Reddit r/photography

---

## 💡 Pro Tips

### Versioning
Use semantic versioning: `MAJOR.MINOR.PATCH`
- v0.1.0 - Initial beta
- v1.0.0 - First stable
- v1.1.0 - New features
- v1.1.1 - Bug fixes

### Auto-updates
Consider implementing:
- Sparkle framework (macOS)
- Check for updates on launch
- In-app update notifications

### Analytics (Optional)
Track (anonymously):
- App launches
- Feature usage
- Crash reports

Use: Firebase, Sentry, or custom

---

## 🆘 Common Issues

### "App is damaged and can't be opened"
```bash
# Remove quarantine attribute
xattr -cr photo_manufactura.app
```

### "Library not loaded"
- Use `macdeployqt` to bundle Qt
- Check with: `otool -L photo_manufactura`

### Large file size
- Normal for bundled Qt app (~500 MB)
- Users prefer working app over small size
- Consider offering "lite" version without AI

---

## 📚 Complete Documentation

For advanced deployment:
- **Full guide**: `docs/DEPLOYMENT.md`
- **System requirements**: `docs/SYSTEM_REQUIREMENTS.md`
- **Build instructions**: `docs/BUILD.md`

---

## 🎉 You're Ready to Ship!

**Simple 3-step deployment:**
```bash
# 1. Build release package
./scripts/build_release.sh

# 2. Test on clean system
# (or ask a friend)

# 3. Upload to GitHub Releases
gh release create v0.1.0 Photo_Manufactura*.dmg
```

**Share your creation with the world! 🌍**
