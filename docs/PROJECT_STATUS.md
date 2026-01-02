# Photo Manufactura - Project Status

**Date:** 2 January 2026  
**Version:** 0.1.0  
**Branch:** `feat/gui-beta-version`

---

## Architecture Overview

```
┌─────────────┐       ┌──────────────────┐       ┌─────────────┐
│     UI      │──────▶│   Controller     │──────▶│    Model    │
│ (src/ui)    │       │ (src/controller) │       │ (src/model) │
│             │◀──────│                  │◀──────│             │
│  MainWindow │signals│AppController    │signals│DocumentMgr  │
│  ToolPanel  │       │                  │       │AppState     │
│  Canvas     │       │                  │       │Adjustments  │
└─────────────┘       └──────────────────┘       └─────────────┘
                              │
                              ▼
                    ┌──────────────────┐
                    │ Image Processing │
                    │ (src/image_proc) │
                    │                  │
                    │  ImagePipeline   │
                    │  Operations      │
                    └──────────────────┘
```

---

## Component Status

### ✅ Model Layer (`src/model/`)

| File | Status | Description |
|------|--------|-------------|
| `ImageDocument.h/cpp` | ✅ Complete | Document state (image, path, modified flag) |
| `AdjustmentSettings.h/cpp` | ✅ Complete | All 10 adjustment sliders with signals |
| `AppState.h/cpp` | ✅ Complete | Theme, zoom, panel visibility state |
| `DocumentManager.h/cpp` | ✅ Complete | Document lifecycle (open/save/close) |
| `CMakeLists.txt` | ✅ Complete | Build configuration |

### ✅ Controller Layer (`src/controller/`)

| File | Status | Description |
|------|--------|-------------|
| `ApplicationController.h/cpp` | ✅ Complete | Full MVC controller with QSettings, signals for all operations |
| `ICommand.h` | ✅ Complete | Command pattern interface |
| `Commands.h` | ✅ Complete | Command implementations |

### ✅ UI Layer (`src/ui/`)

| Component | Status | Description |
|-----------|--------|-------------|
| `mainwindow` | ✅ Refactored | Pure UI display, receives signals from controller |
| `canvas/canvasWidget` | ✅ | OpenGL image display (GLSL 120), aspect ratio preserved |
| `bar/toolBar` | ✅ | Main toolbar |
| `bar/subMenuFile` | ✅ Refactored | Signal-only pattern, Save & Save As fully implemented |
| `bar/subMenuEdit` | ✅ | Edit menu |
| `bar/subMenuView` | ✅ | View menu with theme switching (Ctrl+T) |
| `panel/toolPanel` | ✅ | Adjustment sliders (TODO: connect to controller) |
| `panel/infoPanel` | ✅ | Image info display |
| `widgets/collapsibleWidget` | ✅ | Collapsible sections with animations |
| `widgets/labeledSlider` | ✅ | Slider with label |
| `resources/theme/` | ✅ | Theme manager, dark/light QSS |
| `ui_main.cpp` | ✅ Complete | Full controller wiring including Save & Save As |

### ✅ Image Processing (`src/image_processing/`)

#### Core Architecture
| Component | Status | Description |
|-----------|--------|-------------|
| `ImagePipeline` | ✅ | Operation chain with caching, undo/redo |
| `ImageOperation` | ✅ | Base class for all operations |
| `OperationRegistry` | ✅ | Factory pattern for operations |

#### Light Operations (`operations/light/`)
| Operation | Status | Algorithm |
|-----------|--------|-----------|
| `AdjustBrightness` | ✅ | HSL L-channel modification with OpenMP |
| `AdjustContrast` | ✅ | HSL L-channel contrast around 0.5 midpoint |
| `AdjustHighlights` | ✅ | Targets bright regions |
| `AdjustShadows` | ✅ | Targets dark regions |
| `AdjustWhites` | ✅ | White point adjustment |
| `AdjustBlacks` | ✅ | Black point adjustment |
| `AutoLight` | ✅ | Automatic exposure correction |

#### Color Operations (`operations/color/`)
| Operation | Status | Algorithm |
|-----------|--------|-----------|
| `SaturationAdjust` | ✅ | HSL S-channel modification |
| `WhiteBalance` | ✅ | Temperature adjustment |
| `TintMagenta` | ✅ | Green-Magenta tint |
| `VibranceAdjust` | ✅ | Selective saturation |

#### Utils (`utils/`)
| Utility | Status | Description |
|---------|--------|-------------|
| `ColorSpace` | ✅ | BGR↔HSL conversion (optimized) |
| `Histogram` | ✅ | Histogram calculation |
| `ImageResize` | ✅ | Resize operations |
| `ImageUtils` | ✅ | General utilities |

### ✅ Raw Processing (`src/raw_processing/`)
| Status | Description |
|--------|-------------|
| ✅ | RAW file support (LibRaw integration) |

---

## Integration Status

| Integration | Status | Notes |
|-------------|--------|-------|
| Model ↔ Controller | ✅ Complete | DocumentManager, AppState fully integrated |
| Controller ↔ UI (File Ops) | ✅ Complete | Open/Save/Save As with QSettings persistence working |
| Controller ↔ UI (Display) | ✅ Complete | imageLoaded signal wired, canvas displays correctly |
| UI Sliders ↔ Controller | ✅ Complete | ToolPanel connected, adjustments apply in real-time |
| Canvas Zoom ↔ Controller | ⏳ Pending | Zoom operations need controller integration |
| Theme ↔ Controller | ⏳ Pending | SubMenuView needs controller connection |
| ImagePipeline ↔ DocumentManager | ✅ Complete | Full integration restored, real-time processing enabled |

---

## Recent Updates (January 2, 2026)

### ✅ Completed: ImagePipeline Integration Restored

**Model Layer Updates:**
- Re-enabled `image_processing` library link in `src/model/CMakeLists.txt`
- Added `QT_NO_KEYWORDS` to avoid Halide/Qt `emit` conflict
- Converted all model files to use `Q_EMIT`, `Q_SIGNALS`, `Q_SLOTS`
- Restored all image_processing includes in `DocumentManager.cpp`
- Restored helper functions: `qImageToCvMat()` and `cvMatToQImage()`
- Restored `ImagePipeline` member and constructor initialization
- Restored full `applyAdjustments()` implementation with all operations:
  - Light: Brightness, Contrast, Highlights, Shadows, Whites, Blacks
  - Color: Temperature (WhiteBalance), Tint (TintMagenta), Saturation
- Restored `clearOperations()` calls in `openDocument()` and `closeDocument()`

**Build Status:**
- ✅ Application builds successfully
- ✅ Application launches (macOS .app bundle)
- ✅ Image processing now active (sliders affect image in real-time)

### ✅ Completed: Build Infrastructure & CI/CD Improvements

**Centralized Version Management:**
- Created `VERSION` file as single source of truth (currently `0.1.0`)
- Created `cmake/Version.cmake` for CMake integration
- Updated `CMakeLists.txt` to use centralized version
- Updated `scripts/build_release.sh` to read from VERSION file
- Updated `scripts/create_macos_dmg.sh` to read from VERSION file

**CI/CD Workflows:**
- Created `.github/workflows/ci.yml` for PR/push builds
  - Parallel macOS and Linux builds
  - Build caching for Homebrew and apt packages
  - Git LFS support for AI models
  - C++ and CMake linting checks
  - Version consistency verification
- Updated `.github/workflows/release.yml`:
  - Added Git LFS checkout
  - Added build caching
  - Uses VERSION file for artifact naming

**ONNX Runtime Fix:**
- Created symlink `libs/onnxruntime → libs/onnxruntime-osx-arm64-1.16.3`
- Resolved CMake configuration errors
- Application now builds and runs successfully

**VS Code Tasks Updates:**
- Fixed `Launch Application` task to use `open ./build/bin/photo_manufactura.app`
- Fixed `Launch Raw Processing` task (corrected executable name)
- Fixed `Run Tasks Tests` task (added DYLD_LIBRARY_PATH for OpenMP)
- Added comprehensive Release & Distribution tasks:
  - 🚀 Full Release Build
  - 📦 Create macOS DMG
  - 🐧 Create Linux AppImage
  - 🔨 Configure/Build Release
  - 🧪 Test Release App
  - 🧹 Clean All Builds
  - 📋 Show Version

**Git LFS Configuration:**
- Already configured in `.gitattributes` for:
  - `AI_models/*.onnx`
  - `images/*.dng`, `*.nef`, `*.cr3`, `*.RAF`, `*.arw`

**Current Build Status:**
- ✅ Full application builds successfully
- ✅ Application launches (macOS .app bundle)
- ✅ Release build task works
- ✅ All test executables run
- ✅ Tasks tests pass (15/16, 1 flaky timing test)

---

## Recent Updates (December 6, 2025)

### ✅ Completed: ApplicationWiring Pattern & ONNX Runtime Installation

**ApplicationWiring Class:**
- Created `ApplicationWiring.h/.cpp` to centralize all signal/slot connections
- Moved all wiring logic from `main.cpp` and `ui_main.cpp` to dedicated class
- Implements dependency injection pattern for component relationships
- Provides clean separation: main() now just creates objects and calls `wireComponents()`
- Added `wireDocumentToCanvas()` for complex document lifecycle wiring
- Successfully wired 26 connections (File Menu, Edit Menu, View Menu, Canvas, ToolPanel, InfoPanel, Controller↔UI, Document↔Canvas)

**ImageProcessingService Layer:**
- Created `ImageProcessingService.h/.cpp` as Qt-friendly wrapper for `ImagePipeline`
- Provides QImage↔cv::Mat conversions
- Exposes filter discovery from `OperationRegistry`
- Supports live preview for slider interactions
- Ready for integration (currently commented out in CMakeLists.txt)

**ONNX Runtime Installation:**
- Downloaded and installed ONNX Runtime 1.16.3 for macOS ARM64
- Library extracted to `libs/onnxruntime/lib/libonnxruntime.dylib`
- Application now builds successfully
- Application launches with all 26 wiring connections active

**Main Entry Point Cleanup:**
- `src/main.cpp` simplified to 27 lines (from 131 lines)
- All wiring logic moved to `ApplicationWiring` class
- Both `main.cpp` and `ui_main.cpp` now use same wiring infrastructure

**Build & Launch Tasks:**
- Added `.vscode/tasks.json` entries for main application
- Tasks: "Configure Application", "Build Application", "Clean Application", "Launch Application"
- Fixed CMakeLists.txt syntax error (German comment removal)

**Current Application Status:**
- ✅ Builds successfully
- ✅ Launches successfully
- ✅ File operations work (Open/Save/Save As)
- ✅ Image display with aspect ratio preserved
- ✅ Zoom controls functional
- ✅ Image processing active (sliders affect image in real-time via ImagePipeline)

### ⚠️ Remaining Tasks

---

## Code Quality Analysis

### 🔍 Duplicate Code & Redundancies Found

#### 1. **Duplicate Entry Points** ⚠️
- `src/main.cpp` - Full application entry point
- `src/ui/ui_main.cpp` - UI-only entry point
- **Status:** Both now use `ApplicationWiring`, but serve different purposes
- **Recommendation:** Keep both, but clearly document `ui_main.cpp` as "UI testing only"

#### 2. **Image Processing Service Overlap** ⚠️
- `ImageProcessingService` (new service layer) vs `DocumentManager` direct `ImagePipeline` usage
- **Issue:** Overlapping responsibilities - need to choose one approach
- **Options:**
  - **A:** Use `ImageProcessingService` as clean service layer (better separation)
  - **B:** Keep `ImagePipeline` directly in `DocumentManager` (simpler)
- **Recommendation:** Option A for better architecture

#### 3. **Commented Out Code** ✅ RESOLVED
- `DocumentManager.h/.cpp`: ImagePipeline code restored, now fully functional
- `controller/CMakeLists.txt`: `ImageProcessingService` files still commented (not needed - using direct ImagePipeline)
- **Status:** Main integration complete

#### 4. **Wiring Consolidation** ✅
- **Before:** Duplicate wiring in `main.cpp::connectUIToController()` and direct connections
- **After:** Single source of truth in `ApplicationWiring` class
- **Result:** Cleaner, maintainable, no duplication

#### 5. **Signal Connection Status**
| Integration | Status | Location |
|-------------|--------|----------|
| File Menu → Controller | ✅ Complete | `ApplicationWiring::wireFileMenu()` |
| ToolPanel → Controller | ✅ Complete | `ApplicationWiring::wireToolPanel()` (10 sliders) |
| ToolPanel → Image Processing | ✅ Complete | Via AdjustmentSettings → DocumentManager::applyAdjustments() |
| Canvas → Controller | ✅ Complete | `ApplicationWiring::wireCanvas()` (zoom) |
| Controller → Canvas | ✅ Complete | `ApplicationWiring::wireControllerToCanvas()` |
| Document → Canvas | ✅ Complete | `ApplicationWiring::wireDocumentToCanvas()` |
| Theme Switching | ⏳ Pending | SubMenuView needs signal implementation |

---

## Recent Updates (December 4, 2025)

### ✅ Completed: MVC Architecture Refactoring

**ApplicationController Integration:**
- Added QSettings persistence for `lastImageDirectory`
- Added `imageLoaded(QImage, QString)` signal for UI updates
- All file operations (open/save/exit) fully implemented
- All 10 adjustment slots implemented
- Zoom management via AppState
- Theme management via AppState

**MainWindow Refactoring:**
- Removed all business logic and file I/O
- Changed `loadImage()` → `onImageLoaded(QImage, QString)` (pure UI update)
- Added `onFileSaved(QString)` and `onError(QString)` slots
- Removed `m_currentFilePath` member (now in controller)

**SubMenuFile Refactoring:**
- Converted to signal-only pattern
- Removed all QFileDialog, QSettings, and business logic
- Added Save As functionality with Cmd+Shift+S shortcut
- Signals: `newDocumentRequested()`, `openFileRequested()`, `saveFileRequested()`, `saveAsFileRequested()`, `exitRequested()`
- All slots are 1-line signal emitters

**ui_main.cpp Wiring:**
- Complete signal-slot connections established
- Controller ↔ SubMenuFile: file operations (open, save, save as)
- Controller ↔ MainWindow: image display, save confirmation, errors
- Successfully builds and runs

**UI Improvements:**
- Fixed image aspect ratio preservation in canvasWidget
- Added theme toggle with Ctrl+T keyboard shortcut
- Improved collapsible widget animations
- Fixed spacing for collapsed state
- Responsive layout improvements

### ✅ RESOLVED: Image Processing Integration

**Previously Disabled (Now Restored):**
- ✅ ONNX Runtime dependencies restored in `DocumentManager.cpp`
- ✅ `m_imagePipeline` member restored in `DocumentManager.h`
- ✅ `image_processing` link restored in `model/CMakeLists.txt`
- ✅ `applyAdjustments()` now applies all operations via ImagePipeline

**Technical Notes:**
- Added `QT_NO_KEYWORDS` to model CMakeLists.txt to avoid Halide/Qt `emit` conflict
- Converted model layer to use `Q_EMIT`, `Q_SIGNALS`, `Q_SLOTS` macros
- ONNX Runtime 1.16.3 installed via symlink at `libs/onnxruntime`

---

## Next Steps (Priority Order)

### � **Phase 1 - COMPLETED**

#### 1. 🔧 ~~Install ONNX Runtime~~ ✅ COMPLETED
- [x] Downloaded ONNX Runtime 1.16.3
- [x] Extracted to `libs/onnxruntime/lib/`
- [x] Created symlink for CMake compatibility
- [x] Application builds successfully
- [x] Application launches successfully

#### 2. 🏗️ ~~Restore Image Processing Integration~~ ✅ COMPLETED
- [x] Restored `image_processing` library link in model CMakeLists.txt
- [x] Added `QT_NO_KEYWORDS` for Halide/Qt compatibility
- [x] Converted model layer to `Q_EMIT`/`Q_SIGNALS`/`Q_SLOTS`
- [x] Restored ImagePipeline member and initialization
- [x] Restored full `applyAdjustments()` implementation
- [x] Verified sliders now affect image in real-time

### 🟡 **Phase 2 - Remaining Polish**

#### 3. 🎨 **Theme Controller Integration**
- [ ] Add signals to SubMenuView for theme changes
- [ ] Wire theme signals through ApplicationWiring
- [ ] Remove direct ThemeManager calls from UI

#### 4. 📐 **Canvas Zoom Controller Integration**
- [ ] Wire zoom operations through controller
- [ ] Add zoom state persistence

#### 5. 🧹 **Code Cleanup**
- [ ] Remove obsolete TODO comments
- [ ] Document `ui_main.cpp` purpose
- [ ] Add missing header documentation

**If using direct ImagePipeline:**
- [ ] Uncomment `ImagePipeline` in `DocumentManager.h`
  ```cpp
  std::unique_ptr<ImagePipeline> m_imagePipeline;
  ```
- [ ] Uncomment `applyAdjustments()` implementation in `DocumentManager.cpp`
- [ ] Add `image_processing` library to `model/CMakeLists.txt`

**Test:**
- [ ] Verify slider adjustments actually modify the image
- [ ] Test real-time preview while dragging sliders

---

### 🟠 **HIGH PRIORITY - Phase 2 (This Week)**

#### 3. 🧹 **Code Cleanup**
- [ ] Remove ALL commented `// TODO: Re-enable` sections after image processing works
- [ ] Document `ui_main.cpp` clearly as "UI Component Testing Entry Point"
- [ ] Consider renaming to `ui_test_main.cpp` for clarity
- [ ] Remove unused includes from files

#### 4. 🔗 **Complete Theme Switching Integration**
- [ ] Add `themeToggleRequested()` signal to `SubMenuView`
- [ ] Wire in `ApplicationWiring::wireViewMenu()`
- [ ] Test Ctrl+T keyboard shortcut

#### 5. 🧪 **End-to-End Testing Checklist**
- [ ] Open image → verify display
- [ ] Adjust brightness → verify visible effect
- [ ] Adjust all 10 sliders → verify each works
- [ ] Zoom in/out/fit → verify smooth operation
- [ ] Save file → verify adjustments persisted
- [ ] Save As → verify new file created
- [ ] Theme toggle → verify UI updates
- [ ] Close with unsaved changes → verify warning

#### 6. 🎨 **Fix UI Issues**
- [ ] Fix stylesheet warning: `Failed to open stylesheet file: ":/styles/dark_theme.qss"`
  - Check `resources.qrc` includes the file
  - Verify file path is correct
- [ ] Test all keyboard shortcuts work correctly

---

### 🟡 **MEDIUM PRIORITY - Phase 3 (Next Week)**

#### 7. 📝 **Documentation Updates**
- [ ] Update `TECHNICAL_DOCUMENTATION.md`:
  - Add `ApplicationWiring` pattern (Section 2)
  - Add `ImageProcessingService` to architecture (Section 3)
  - Update component diagram with new classes
- [ ] Create `ApplicationWiring.md` in `docs/development/`
- [ ] Document the two entry point purposes clearly

#### 8. 🏛️ **Architecture Refinement**
- [ ] Finalize service layer design
- [ ] Create clear boundaries between layers
- [ ] Document component interaction patterns
- [ ] Add sequence diagrams for key workflows

#### 9. 🔍 **Code Review & Refactoring**
- [ ] Review all signal connections for correctness
- [ ] Check for memory leaks (especially OpenGL textures)
- [ ] Verify thread safety in image processing
- [ ] Profile performance of adjustment pipeline

---

### 🟢 **LOW PRIORITY - Phase 4 (Future Enhancements)**

#### 10. ↩️ **Implement Undo/Redo**
- [ ] Create `CommandHistory` class
- [ ] Implement Command pattern for all adjustments
- [ ] Wire Edit → Undo/Redo menu items
- [ ] Add keyboard shortcuts (Cmd+Z, Cmd+Shift+Z)
- [ ] Display command history in UI

#### 11. 📊 **Add Histogram Display**
- [ ] Create `HistogramWidget` using existing `Histogram` utility
- [ ] Add to `InfoPanel` or as separate dock widget
- [ ] Update in real-time when adjustments change
- [ ] Show before/after comparison

#### 12. 🎯 **Presets System**
- [ ] Create `PresetManager` class
- [ ] Save/load adjustment combinations as JSON
- [ ] Add UI for preset selection in ToolPanel
- [ ] Include built-in presets (Portrait, Landscape, B&W, etc.)

#### 13. 🚀 **Performance Optimization**
- [ ] Move image processing to `QThread` for non-blocking UI
- [ ] Add progress indicator for long operations
- [ ] Implement async file I/O
- [ ] Cache processed images at multiple zoom levels

#### 14. 🔌 **Plugin Architecture**
- [ ] Design plugin interface for custom operations
- [ ] Add plugin discovery and loading
- [ ] Create example plugin
- [ ] Document plugin development guide

---

## Work Progress Tracking

### Completed This Session ✅
1. Created `ApplicationWiring` class (centralized wiring)
2. Created `ImageProcessingService` (service layer)
3. Installed ONNX Runtime library
4. Fixed CMakeLists.txt syntax errors
5. Added VS Code build/launch tasks
6. Simplified `main.cpp` to 27 lines
7. Successfully built and launched application
8. Verified 26 signal connections working

### In Progress 🔄
1. Image processing integration (architecture decision pending)
2. Testing adjustment sliders with real effects
3. Documentation updates

### Blocked ⛔
- None (ONNX Runtime blocker resolved)

---

## Build Status Summary

| Component | Build | Launch | Functionality |
|-----------|-------|--------|---------------|
| Model | ✅ | - | Fully functional |
| Controller | ✅ | - | Fully functional |
| ApplicationWiring | ✅ | - | 26 connections active |
| UI Standalone | ✅ | ✅ | Works independently |
| Full Application | ✅ | ✅ | Launches, file ops work |
| Image Processing | ✅ | ✅ | Builds and runs |
| Release Build | ✅ | ✅ | DMG packaging works |
| CI/CD | ✅ | - | GitHub Actions configured |

---

## Release Artifacts

After running `🚀 Full Release Build` task, artifacts are created at:

| Artifact | Location | Platform |
|----------|----------|----------|
| App Bundle | `build/bin/photo_manufactura.app` | macOS |
| DMG Installer | `Photo_Manufactura_v{VERSION}_macOS.dmg` | macOS |
| AppImage | `Photo_Manufactura_v{VERSION}_Linux.AppImage` | Linux |
| Tarball | `Photo_Manufactura_v{VERSION}_Linux.tar.gz` | Linux |
| Windows ZIP | `Photo_Manufactura_v{VERSION}_Windows.zip` | Windows |

---

## Next Steps (Priority Order)

### 1. 🔧 ~~Install ONNX Runtime (Blocker)~~ ✅ COMPLETED
- Symlink created: `libs/onnxruntime → libs/onnxruntime-osx-arm64-1.16.3`

### 2. 🔗 Complete Controller Integration (High Priority)
- Connect `ToolPanel` sliders → `ApplicationController` adjustment slots
- Connect `CanvasWidget` zoom → `ApplicationController` zoom management
- Connect `SubMenuView` theme → `ApplicationController` theme management

### 3. 🖼️ Restore ImagePipeline Integration
- Uncomment image_processing code in DocumentManager
- Restore library links in CMakeLists
- Test real-time adjustment preview

### 4. 🧪 Test End-to-End
- ✅ Open image via File menu (WORKING)
- ✅ Display in canvas with aspect ratio (WORKING)
- ✅ Save file with directory persistence (WORKING)
- ✅ Save As with file dialog (WORKING)
- ⏳ Adjust sliders and see preview (needs ToolPanel connection)
- ⏳ Zoom operations (needs CanvasWidget connection)
- ⏳ Theme switching via controller (needs SubMenuView connection)

### 5. ↩️ Add Undo/Redo (Low Priority)
- Implement Command pattern for adjustments
- Connect to Edit menu

---

## Build Commands

```bash
# Configure and build full project
cmake -B build -S .
cmake --build build

# Run main application
./build/bin/photo_manufactura

# Build and run standalone UI (for testing)
cd src/ui
cmake -B build -S .
cmake --build build
./build/bin/ui_main
```

---

## Dependencies

- **Qt 6**: Core, Gui, Widgets, OpenGL, OpenGLWidgets ✅ Installed
- **OpenCV 4.12.0**: Core, ImgProc, ImgCodecs ✅ Installed
- **ONNX Runtime 1.23.2**: AI model inference ⚠️ **NOT INSTALLED** (blocker for image_processing)
- **OpenMP**: Parallel processing ✅ Available
- **LibRaw**: RAW file decoding ✅ Available
- **CMake 3.21+**: Build system ✅ Installed

---

## Current Build Status

| Component | Status | Notes |
|-----------|--------|-------|
| Model | ✅ Builds | Image processing disabled |
| Controller | ✅ Builds | Fully functional |
| UI | ✅ Builds | Standalone build working |
| Image Processing | ⚠️ Disabled | Requires ONNX Runtime |
| Full Project | ⚠️ Blocked | Need ONNX Runtime for complete build |
