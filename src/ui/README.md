# UI Component - Photo Manufactura

## Overview

The UI component provides the graphical user interface for Photo Manufactura, a Lightroom-inspired photo editing application. It features a modern dark theme, collapsible panels, and a professional editing workspace.

---

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

## Component Details

### 🖼️ Main Window (`mainwindow.h/cpp`)

The central application window that orchestrates all UI components.

**Features:**
- Menu bar with File, Edit, and View menus
- Dockable panels (adjustments and info)
- Central canvas for image display
- Theme management integration

**Key Methods:**
```cpp
MainWindow(QWidget* parent = nullptr);
CanvasWidget* getCanvasWidget() const;
```

---

### 📋 Menu Bar Components (`bar/`)

#### **SubMenuFile** - File Operations
- **New**: Create new document
- **Open**: Load images (PNG, JPG, RAW, etc.)
- **Save**: Export edited image
- **Exit**: Close application
- Keyboard shortcuts: `Ctrl+N`, `Ctrl+O`, `Ctrl+S`, `Ctrl+Q`

#### **SubMenuEdit** - Editing Commands
- **Undo**: Revert last change (`Ctrl+Z`)
- **Redo**: Reapply change (`Ctrl+Shift+Z`)
- TODO: Copy/Paste adjustments

#### **SubMenuView** - View Controls
- **Theme Switcher**: Dark/Light theme
- **Panel Toggles**: Show/hide tool and info panels
- Keyboard shortcuts: `T` (tools), `I` (info)

---

### 🎛️ Side Panels (`panel/`)

#### **ToolPanel** - Adjustment Controls
Left-side panel with collapsible sections for image adjustments.

**Sections:**
1. **Basic Adjustments**
   - Exposure (-100 to +100)
   - Contrast (-100 to +100)
   - Highlights (-100 to +100)
   - Shadows (-100 to +100)
   - Whites (-100 to +100)
   - Blacks (-100 to +100)

2. **Color Adjustments**
   - Temperature (-100 to +100)
   - Tint (-100 to +100)
   - Saturation (-100 to +100)

3. **Detail**
   - Brightness (-100 to +100)
   - TODO: Sharpness, Noise Reduction

**Features:**
- Reset All button
- Scrollable for unlimited controls
- Real-time value display

#### **InfoPanel** - Image Information
Right-side panel displaying image metadata.

**Displays:**
- File path and name
- Dimensions (width × height)
- File format
- Histogram placeholder
- TODO: EXIF data, color space info

---

### 🧩 Custom Widgets (`widgets/`)

#### **CanvasWidget** - Image Display
OpenGL-accelerated widget for rendering images.

**Features:**
- Hardware-accelerated rendering
- Pan: Click and drag
- Zoom: Mouse wheel
- Zoom to fit/100% view
- TODO: Grid overlay, crop tools

**Signals:**
```cpp
void imageClicked(QPoint position);
void zoomChanged(double factor);
```

#### **CollapsibleWidget** - Expandable Sections
Animated collapsible container for organizing controls.

**Features:**
- Smooth expand/collapse animation (300ms)
- Arrow indicator
- Click title to toggle
- Scrollable content

**Usage:**
```cpp
CollapsibleWidget* section = new CollapsibleWidget("Title", parent);
QVBoxLayout* layout = new QVBoxLayout();
layout->addWidget(slider1);
section->setContentLayout(layout);
```

#### **LabeledSlider** - Combined Control
Slider with label and numeric input for precise adjustments.

**Features:**
- Label above control
- Horizontal slider
- SpinBox for precise input
- Bi-directional sync (slider ↔ spinbox)
- Reset to default value

**Signals:**
```cpp
void valueChanged(int value);
```

#### **StyleSheet** - Theme Loader
Utility class for loading QSS stylesheets from resources or files.

**Methods:**
```cpp
static QString loadTheme(Theme theme);
static QString loadFromFile(const QString& filePath);
static QString getDarkTheme();
static QString getLightTheme();
```

#### **ThemeManager** - Theme Control
Singleton for managing application-wide themes.

**Features:**
- Centralized theme management
- Theme persistence (QSettings)
- Theme change notifications
- Support for Dark/Light/Auto themes

**Usage:**
```cpp
// Apply theme
ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);

// Listen for changes
connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
        this, &MyWidget::onThemeChanged);
```

---

### 🎨 Themes & Styling (`resources/`)

#### **Dark Theme** (`dark_theme.qss`)
Professional dark theme inspired by Adobe Lightroom.

**Colors:**
- Background: `#1e1e1e`
- Panels: `#252525`
- Borders: `#3e3e3e`
- Text: `#cccccc`
- Accent: `#0078d4` (blue)

#### **Light Theme** (`light_theme.qss`)
Clean light theme for daytime editing.

**Colors:**
- Background: `#f5f5f5`
- Panels: `#ffffff`
- Borders: `#d0d0d0`
- Text: `#333333`
- Accent: `#0078d4` (blue)

#### **Qt Resource System** (`resources.qrc`)
Embeds QSS files into the executable for:
- No external file dependencies
- Fast loading
- Cross-platform compatibility
- Access via `:/styles/` prefix

---

## Building

### Standalone Build
```bash
cd src/ui
cmake -B build -S .
cmake --build build
./build/bin/ui_main
```

### As Submodule
```cmake
add_subdirectory(src/ui)
target_link_libraries(your_app PRIVATE ui)
```

---

## Dependencies

### Required Qt6 Components
- **Qt6::Core** - Core functionality
- **Qt6::Gui** - GUI support
- **Qt6::Widgets** - Widget classes
- **Qt6::OpenGL** - OpenGL support
- **Qt6::OpenGLWidgets** - OpenGL widget integration

### CMake Requirements
- CMake 3.21+
- C++20 compiler
- Qt 6.x

---

## Architecture

### Design Pattern
The UI follows the **MVC (Model-View-Controller)** pattern:
- **View**: UI components (this module)
- **Controller**: `ApplicationController` (parent module)
- **Model**: `Document`, `ImagePipeline` (parent modules)

### Communication
```
┌─────────────┐
│ MainWindow  │ ← User interaction
└──────┬──────┘
       │
       ├─→ ToolPanel ──signals──→ ApplicationController
       ├─→ InfoPanel
       └─→ CanvasWidget ──signals──→ ApplicationController
                                         │
                                         ↓
                                   ImagePipeline
```

---

## Key Features

### ✅ Implemented
- Professional dark/light themes
- Collapsible adjustment panels
- Custom slider widgets with precise input
- Menu bar with keyboard shortcuts
- Dockable/floatable panels
- Theme persistence
- OpenGL-accelerated canvas
- Responsive layout

### 🚧 TODO
- Histogram display implementation
- EXIF metadata viewer
- Before/after comparison view
- Preset system (save/load adjustments)
- Color picker tool
- Crop/rotate tools
- Multiple image support (filmstrip)
- Full-screen mode
- Zoom navigator mini-map

---

## Development Guidelines

### Adding New Adjustments
1. Create slider in `ToolPanel::createXXXSection()`
2. Add signal to `ToolPanel` header
3. Connect signal in section creator
4. Update `resetAllAdjustments()` method

### Creating New Widgets
1. Add `.h` and `.cpp` files to `widgets/`
2. Update `CMakeLists.txt` (WIDGET_SOURCES)
3. Follow naming convention: `camelCase` files, `PascalCase` classes
4. Inherit from appropriate Qt base class
5. Use signals/slots for communication

### Styling Guidelines
- Edit `.qss` files for theme changes
- Use consistent color variables
- Test both dark and light themes
- Follow Qt stylesheet syntax
- Use class selectors for specificity

### Testing
```bash
# Run standalone UI
./build/bin/ui_main

# Hot reload during development
# Add Ctrl+R shortcut to reload stylesheets
```

---

## Performance Considerations

### Canvas Widget
- OpenGL for hardware acceleration
- Debounce slider events (50ms)
- Avoid unnecessary redraws

### Theme Switching
- Stylesheets cached in memory
- Theme applied globally via `qApp`
- Minimal overhead on switch

### Panel Updates
- Lazy loading of adjustment groups
- Only update visible sliders
- Batch updates when multiple changes

---

## Integration Points

### With Controller
```cpp
// MainWindow provides access to canvas
CanvasWidget* canvas = mainWindow->getCanvasWidget();
canvas->setImage(image);

// Panels emit signals for adjustments
connect(toolPanel, &ToolPanel::brightnessChanged,
        controller, &ApplicationController::onBrightnessChanged);
```

### With Image Processing
```cpp
// Controller forwards to image pipeline
void ApplicationController::onBrightnessChanged(int value) {
    m_imagePipeline->adjustBrightness(value);
    updateCanvas();
}
```

---

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+N` | New document |
| `Ctrl+O` | Open image |
| `Ctrl+S` | Save image |
| `Ctrl+Q` | Exit application |
| `Ctrl+Z` | Undo |
| `Ctrl+Shift+Z` | Redo |
| `T` | Toggle tool panel |
| `I` | Toggle info panel |

---

## Resources

- **Documentation**: `docs/UI_WIDGETS_GUIDE.md`
- **QSS Guide**: `docs/QSS_REFACTORING_GUIDE.md`
- **Qt Docs**: https://doc.qt.io/qt-6/

---

## License

Part of Photo Manufactura project. See parent LICENSE file.

---

## Maintainers

UI Component maintained as part of the Photo Manufactura project.

**Last Updated**: November 23, 2025
