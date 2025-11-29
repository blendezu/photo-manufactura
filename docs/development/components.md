# Component Development Guide

Architecture and design patterns for Photo Manufactura components.

## MVC Architecture

```
photo_manufactura (main)
├── model           # Application state (ImageDocument, Settings)
├── controller      # Business logic orchestration
├── ui              # Qt6 user interface (View)
├── image_processing # Core algorithms (ImagePipeline)
├── raw_processing  # RAW file handling
└── scheduler_worker # Task management
```

> **See also**: [Model Layer](Model.md) for detailed Model documentation.

Each component is:
- **Self-contained** - builds independently
- **Loosely coupled** - minimal dependencies  
- **Testable** - has own test suite

## Component Layout

```
src/component_name/
├── CMakeLists.txt        # Build config
├── CMakePresets.json     # Presets
├── component.h/cpp       # Implementation
├── test_main.cpp         # Tests
└── build/                # Build output
```

## Qt Components

### Class Structure

```cpp
#pragma once
#include <QWidget>

class ComponentWidget : public QWidget {
    Q_OBJECT  // Required for signals/slots
    
public:
    explicit ComponentWidget(QWidget* parent = nullptr);
    
private slots:
    void onButtonClicked();
};
```

### CMake Setup

```cmake
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
target_link_libraries(component PUBLIC Qt6::Core Qt6::Widgets)
```

## Design Patterns

### Interfaces

```cpp
class IImageProcessor {
public:
    virtual ~IImageProcessor() = default;
    virtual ProcessResult process(const Image& input) = 0;
};

class ImageProcessor : public IImageProcessor {
    ProcessResult process(const Image& input) override { /* ... */ }
};
```

### Dependency Injection

```cpp
class ApplicationController {
    std::unique_ptr<IImageProcessor> processor_;
    std::unique_ptr<IUserInterface> ui_;
    
public:
    ApplicationController(
        std::unique_ptr<IImageProcessor> processor,
        std::unique_ptr<IUserInterface> ui
    );
};
```

### Qt Signals/Slots (Preferred)

```cpp
// Model emits signals when data changes
class ImageDocument : public QObject {
    Q_OBJECT
signals:
    void imageChanged(const QImage& image);
    void modifiedChanged(bool modified);
};

// Controller connects Model to View
connect(document, &ImageDocument::imageChanged,
        canvas, &CanvasWidget::updateImage);
```

## Best Practices

### RAII Resources

```cpp
class Resource {
public:
    Resource() { /* acquire */ }
    ~Resource() { /* release */ }
    Resource(const Resource&) = delete;
    Resource(Resource&&) = default;
};
```

### Error Handling

```cpp
// C++23 expected
std::expected<Result, ErrorCode> process(const Input& input) {
    if (input.empty()) return std::unexpected(ErrorCode::EMPTY);
    return Result{};
}
```

### Documentation

```cpp
/**
 * @brief Process image with parameters
 * @param input Input image
 * @param params Processing parameters
 * @return ProcessResult or error
 */
ProcessResult process(const Image& input, const Params& params = {});
```

## Theme System

The UI uses external QSS files for theming, managed via `ThemeManager` singleton.

### File Structure
```
src/ui/resources/
├── resources.qrc           # Qt resources (prefix: /styles)
├── styles/
│   ├── dark_theme.qss
│   └── light_theme.qss
└── theme/
    ├── themeManager.h/cpp  # Singleton for theme switching
    └── styleSheet.h/cpp    # QSS file loader
```

### Usage
```cpp
// Apply theme globally
ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);

// Load QSS from resources
QString qss = StyleSheet::loadQssFile(":/styles/dark_theme.qss");
```

### Adding New Themes
1. Create `new_theme.qss` in `resources/styles/`
2. Add to `resources.qrc`: `<file>styles/new_theme.qss</file>`
3. Add enum value and case in `ThemeManager`
