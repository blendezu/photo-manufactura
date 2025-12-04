# Photo Manufactura - Project Status

**Date:** 4 December 2025  
**Branch:** `feat/gui-core-components`

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
| UI Sliders ↔ Controller | ⏳ Pending | ToolPanel needs signal connections |
| Canvas Zoom ↔ Controller | ⏳ Pending | Zoom operations need controller integration |
| Theme ↔ Controller | ⏳ Pending | SubMenuView needs controller connection |
| ImagePipeline ↔ DocumentManager | ⚠️ Disabled | Temporarily disabled (requires ONNX Runtime) |

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

### ⚠️ Temporary Changes for Build

**Image Processing Disabled:**
- Commented out ONNX Runtime dependencies in `DocumentManager.cpp`
- Removed `m_imagePipeline` member from `DocumentManager.h`
- Removed `image_processing` link from `model/CMakeLists.txt`
- `applyAdjustments()` currently returns original image unchanged

**Reason:** ONNX Runtime library not installed at `/libs/onnxruntime/lib/`

**To Restore Full Functionality:**
1. Install ONNX Runtime 1.23.2 for macOS ARM64
2. Uncomment all `// TODO: Re-enable when image_processing is available` sections
3. Restore `image_processing` link in CMakeLists
4. Rebuild project

---

## Next Steps (Priority Order)

### 1. 🔧 Install ONNX Runtime (Blocker)
```bash
cd libs/onnxruntime
curl -L -o onnxruntime.tgz https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-osx-arm64-1.23.2.tgz
tar -xzf onnxruntime.tgz
mkdir -p lib
cp onnxruntime-osx-arm64-1.23.2/lib/* lib/
```

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
