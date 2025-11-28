# Component Development Guide

Architecture and design patterns for Photo Manufactura components.

## Architecture

```
photo_manufactura (main)
├── ui              # Qt6 user interface
├── controller      # Application logic
├── image_processing # Core algorithms
├── raw_processing  # RAW file handling
└── scheduler_worker # Task management
```

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

### Event Bus

```cpp
class EventBus {
public:
    template<typename Event>
    void subscribe(std::function<void(const Event&)> handler);
    
    template<typename Event>
    void publish(const Event& event);
};
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
