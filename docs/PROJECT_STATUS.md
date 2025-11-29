# Photo Manufactura - Project Status

**Date:** 29 November 2025  
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
| `ApplicationController.h/cpp` | ✅ Updated | Uses Model layer, all adjustment slots |
| `ICommand.h` | ✅ Complete | Command pattern interface |
| `Commands.h` | ✅ Complete | Command implementations |

### ✅ UI Layer (`src/ui/`)

| Component | Status | Description |
|-----------|--------|-------------|
| `mainwindow` | ✅ | Main application window |
| `canvas/canvasWidget` | ✅ | OpenGL image display (GLSL 120) |
| `bar/toolBar` | ✅ | Main toolbar |
| `bar/subMenuFile` | ✅ | File menu (TODO: wire to controller) |
| `bar/subMenuEdit` | ✅ | Edit menu |
| `bar/subMenuView` | ✅ | View menu with theme switching |
| `panel/toolPanel` | ✅ | Adjustment sliders (TODO: wire to controller) |
| `panel/infoPanel` | ✅ | Image info display |
| `widgets/collapsibleWidget` | ✅ | Collapsible sections |
| `widgets/labeledSlider` | ✅ | Slider with label |
| `resources/theme/` | ✅ | Theme manager, dark/light QSS |

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
| Model ↔ Controller | ✅ | DocumentManager, AppState connected |
| Controller ↔ UI | ⏳ Pending | Signals not yet connected |
| ImagePipeline ↔ DocumentManager | ⏳ Pending | Placeholder in `applyAdjustments()` |
| UI Sliders ↔ Controller | ⏳ Pending | Need to emit signals |
| Canvas ↔ ProcessedImage | ⏳ Pending | Need to display from Model |

---

## Next Steps (Priority Order)

### 1. 🔗 Wire UI to Controller (High Priority)
- Connect `ToolPanel` sliders → `ApplicationController` adjustment slots
- Connect `SubMenuFile` actions → `ApplicationController.openFile()`, etc.
- Connect `CanvasWidget` → display `DocumentManager.processedImage()`

### 2. 🖼️ Integrate ImagePipeline (High Priority)
- Replace placeholder in `DocumentManager::applyAdjustments()`
- Create operations from `AdjustmentSettings` values
- Process images in real-time as sliders change

### 3. 🧪 Test End-to-End
- Open image via File menu
- Adjust sliders and see preview
- Save processed image

### 4. ↩️ Add Undo/Redo (Low Priority)
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

- **Qt 6**: Core, Gui, Widgets, OpenGL, OpenGLWidgets
- **OpenCV 4**: Core, ImgProc, ImgCodecs, HighGui
- **OpenMP**: Parallel processing
- **LibRaw**: RAW file decoding
- **CMake 3.21+**: Build system
