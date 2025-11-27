# UI Component Overview - Photo Manufactura

## Directory Structure

```
src/ui/
├── CMakeLists.txt              # Build configuration
├── CMakePresets.json           # CMake presets
├── mainwindow.h/cpp            # Main application window
├── ui_main.cpp                 # Standalone executable entry point
│
├── bar/                        # Menu bar components
│   ├── toolBar.h/cpp          # Main toolbar (legacy)
│   ├── subMenuFile.h/cpp      # File menu (New, Open, Save, Exit)
│   ├── subMenuEdit.h/cpp      # Edit menu (Undo, Redo)
│   └── subMenuView.h/cpp      # View menu (Theme switcher, panel toggles)
│
├── panel/                      # Dockable side panels
│   ├── toolPanel.h/cpp        # Adjustment controls (left panel)
│   └── infoPanel.h/cpp        # Image metadata & histogram (right panel)
│
├── widgets/                    # Custom reusable widgets
│   ├── canvasWidget.h/cpp     # OpenGL image display widget
│   ├── collapsibleWidget.h/cpp # Expandable/collapsible sections
│   ├── labeledSlider.h/cpp    # Slider with label and spinbox
│   ├── styleSheet.h/cpp       # QSS stylesheet loader
│   └── themeManager.h/cpp     # Theme management singleton
│
├── resources/                  # Qt resources
│   ├── resources.qrc          # Resource collection file
│   └── styles/                # QSS stylesheets
│       ├── dark_theme.qss     # Dark theme (default)
│       └── light_theme.qss    # Light theme
│
├── test/                       # Unit tests (future)
└── build/                      # Build output directory
```

---

## Component Summary

| Component | Purpose | Files |
|-----------|---------|-------|
| **Main Window** | Application orchestrator | `mainwindow.h/cpp` |
| **Menu Bar** | File, Edit, View menus | `bar/*.h/cpp` |
| **Tool Panel** | Adjustment controls (left) | `panel/toolPanel.h/cpp` |
| **Info Panel** | Metadata display (right) | `panel/infoPanel.h/cpp` |
| **Canvas** | OpenGL image viewer | `widgets/canvasWidget.h/cpp` |
| **Collapsible** | Expandable sections | `widgets/collapsibleWidget.h/cpp` |
| **Labeled Slider** | Slider + SpinBox control | `widgets/labeledSlider.h/cpp` |
| **Theme System** | Dark/Light themes | `widgets/styleSheet.h/cpp`, `widgets/themeManager.h/cpp` |
| **Resources** | QSS stylesheets | `resources/*.qrc`, `resources/styles/*.qss` |

---

## Quick Reference

### Building
```bash
# Standalone
cd src/ui && cmake -B build && cmake --build build
./build/bin/ui_main

# As library
add_subdirectory(src/ui)
target_link_libraries(your_app PRIVATE ui)
```

### Key Classes
```cpp
// Main window
MainWindow* window = new MainWindow();
CanvasWidget* canvas = window->getCanvasWidget();

// Theme management
ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);

// Custom widgets
CollapsibleWidget* section = new CollapsibleWidget("Title");
LabeledSlider* slider = new LabeledSlider("Brightness", -100, 100);
```

### Signals
```cpp
// ToolPanel adjustments
void brightnessChanged(int value);
void contrastChanged(int value);
void exposureChanged(int value);
// ... etc

// CanvasWidget interactions
void imageClicked(QPoint position);
void zoomChanged(double factor);

// ThemeManager changes
void themeChanged(Theme theme);
```

---

## Architecture

```
┌──────────────────────────────────────────────────────┐
│                    MainWindow                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │  File Edit  │  │CanvasWidget │  │  Tool Panel │  │
│  │  View Help  │  │   (OpenGL)  │  │  Info Panel │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
└──────────────────────────────────────────────────────┘
           │                │                │
           └────────────────┴────────────────┘
                           │
                    signals/slots
                           │
                           ↓
              ┌─────────────────────────┐
              │  ApplicationController  │
              └─────────────────────────┘
                           │
                           ↓
              ┌─────────────────────────┐
              │    ImagePipeline        │
              └─────────────────────────┘
```

---

## Dependencies

- **Qt6::Core** - Core functionality
- **Qt6::Gui** - GUI support
- **Qt6::Widgets** - Widget classes
- **Qt6::OpenGL** - OpenGL support
- **Qt6::OpenGLWidgets** - OpenGL widgets

---

## Status

### ✅ Implemented
- Dark/Light themes with QSS files
- Collapsible adjustment panels
- Custom slider widgets
- Menu bar with shortcuts
- Dockable panels
- Theme persistence
- OpenGL canvas

### 🚧 In Progress
- Histogram display
- EXIF metadata viewer
- Before/after comparison

### 📋 Planned
- Preset system
- Crop/rotate tools
- Color picker
- Multiple images
- Full-screen mode

---

## File Count

- **Headers**: 10 files
- **Sources**: 10 files
- **QSS**: 2 files
- **Resources**: 1 file
- **Total**: 23 files

---

For detailed documentation, see:
- **Full Overview**: `src/ui/README.md`
- **Widget Guide**: `docs/UI_WIDGETS_GUIDE.md`
- **QSS Refactoring**: `docs/QSS_REFACTORING_GUIDE.md`
