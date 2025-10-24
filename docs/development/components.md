# 🧩 Component Development Guide

## 📋 Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Component Structure](#component-structure)
3. [Development Workflow](#development-workflow)
4. [Qt Component Guidelines](#qt-component-guidelines)
5. [Testing Strategy](#testing-strategy)
6. [Integration Patterns](#integration-patterns)
7. [Best Practices](#best-practices)

## 🏛️ Architecture Overview

Photo Manufactura uses a **modular component architecture** where each component is:

- ✅ **Self-contained** - Can be built and tested independently
- ✅ **Reusable** - Components can be libraries or executables
- ✅ **Loosely coupled** - Minimal dependencies between components
- ✅ **Testable** - Each component has its own test suite

### Component Hierarchy

```
📦 photo_manufactura (main executable)
├── 🖼️  ui (Qt6 user interface)
├── 🎮 controller (application logic)
├── 🎨 image_processing (core algorithms)
├── 📸 raw_processing (RAW file handling)
└── ⚡ scheduler_worker (task management)
```

## 📁 Component Structure

### Standard Component Layout

```
src/component_name/
├── 📄 CMakeLists.txt           # Build configuration
├── 🔧 CMakePresets.json        # Component-specific presets
├── 📦 component_main.cpp/.h    # Main implementation
├── 🧪 test_component.cpp       # Component tests
├── 🔨 build/                   # Build artifacts (generated)
│   ├── bin/test_component      # Test executable
│   └── libcomponent.a          # Static library
└── 📚 docs/                    # Component documentation
```

### CMakeLists.txt Template

```cmake
# Component Name - Independent and integrated builds
# Check if standalone build
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.21)
    project(component_name VERSION 1.0.0 LANGUAGES CXX)
    
    # Standalone settings
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    
    # Qt components (if needed)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# Create component library
add_library(component_name STATIC)

# Add source files
target_sources(component_name PRIVATE
    component_main.cpp
    component_main.h
    # Add more sources here
)

# Include directories
target_include_directories(component_name PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Dependencies
find_package(Qt6 COMPONENTS Core Widgets)
if(Qt6_FOUND)
    target_link_libraries(component_name PUBLIC Qt6::Core Qt6::Widgets)
    message(STATUS "Qt6 found for component_name: ${Qt6_VERSION}")
else()
    message(WARNING "Qt6 not found for component_name")
endif()

# Test executable (standalone only)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(test_component)
    
    target_sources(test_component PRIVATE
        test_component.cpp
    )
    
    target_link_libraries(test_component PRIVATE component_name)
    
    set_target_properties(test_component PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()
```

## 🚀 Development Workflow

### 1. Creating a New Component

```bash
# 1. Create component directory
mkdir src/new_component
cd src/new_component

# 2. Copy template files
cp ../ui/CMakeLists.txt ./
cp ../ui/CMakePresets.json ./

# 3. Customize for your component
# Edit CMakeLists.txt, CMakePresets.json
# Create source files

# 4. Test standalone build
cmake --preset debug
cmake --build --preset debug
./build/bin/test_new_component
```

### 2. Development Cycle

```bash
# Development loop
while developing:
    # 1. Edit component source files
    vim component_main.cpp
    
    # 2. Quick component build
    cmake --build --preset debug
    
    # 3. Test component
    ./build/bin/test_component
    
    # 4. Integration test (periodically)
    cd ../../  # Back to project root
    cmake --build --preset dev
    ./build/dev/bin/photo_manufactura
```

### 3. VS Code Integration

```bash
# Method 1: Multi-root workspace
# Open photo-manufactura.code-workspace
# Switch to component folder in Explorer

# Method 2: Tasks
Cmd+Shift+P → Tasks: Run Task → Build [Component] Component

# Method 3: Status bar
# Click folder name → Select component → Click build
```

## 🖼️ Qt Component Guidelines

### Qt-Specific Setup

For components using Qt (like UI):

```cmake
# Essential Qt settings
set(CMAKE_AUTOMOC ON)      # Meta-Object Compiler
set(CMAKE_AUTORCC ON)      # Resource Compiler  
set(CMAKE_AUTOUIC ON)      # UI Compiler

# Qt dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
target_link_libraries(component_name PUBLIC 
    Qt6::Core 
    Qt6::Widgets
)
```

### Qt Class Structure

```cpp
// component.h
#pragma once
#include <QWidget>

class ComponentWidget : public QWidget {
    Q_OBJECT  // Required for slots/signals
    
public:
    explicit ComponentWidget(QWidget* parent = nullptr);
    
private slots:
    void onButtonClicked();
    
private:
    // Private members
};
```

### Qt Test Pattern

```cpp
// test_component.cpp
#include "component.h"
#include <QApplication>
#include <QTest>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    ComponentWidget widget;
    widget.show();
    
    // Basic functionality test
    // Return 0 for success, 1 for failure
    return 0;
}
```

## 🧪 Testing Strategy

### Component Test Types

1. **Unit Tests** - Individual function testing
2. **Integration Tests** - Component interaction
3. **UI Tests** - Qt widget functionality
4. **Performance Tests** - Benchmark critical paths

### Test Implementation

```cpp
// test_component.cpp
#include "component_main.h"
#include <iostream>
#include <cassert>

void test_basic_functionality() {
    ComponentClass component;
    
    // Test initialization
    assert(component.isInitialized());
    
    // Test basic operations
    auto result = component.process("test input");
    assert(!result.empty());
    
    std::cout << "✅ Basic functionality test passed\n";
}

void test_error_handling() {
    ComponentClass component;
    
    // Test error conditions
    try {
        component.process("");  // Should handle empty input
        std::cout << "✅ Error handling test passed\n";
    } catch (const std::exception& e) {
        std::cout << "❌ Unexpected exception: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "Running Component Tests...\n";
    
    test_basic_functionality();
    test_error_handling();
    
    std::cout << "🎉 All tests passed!\n";
    return 0;
}
```

### Test Automation

```cmake
# Enable testing in standalone mode
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    enable_testing()
    
    # Add test
    add_test(NAME component_tests COMMAND test_component)
    
    # Set test properties
    set_tests_properties(component_tests PROPERTIES
        TIMEOUT 30
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
```

## 🔄 Integration Patterns

### Component Communication

```cpp
// Use interfaces for loose coupling
class IImageProcessor {
public:
    virtual ~IImageProcessor() = default;
    virtual ProcessResult process(const Image& input) = 0;
};

// Component implements interface
class ImageProcessingComponent : public IImageProcessor {
public:
    ProcessResult process(const Image& input) override {
        // Implementation
    }
};
```

### Dependency Injection

```cpp
// Controller coordinates components
class ApplicationController {
private:
    std::unique_ptr<IImageProcessor> processor_;
    std::unique_ptr<IUserInterface> ui_;
    
public:
    ApplicationController(
        std::unique_ptr<IImageProcessor> processor,
        std::unique_ptr<IUserInterface> ui
    ) : processor_(std::move(processor)), ui_(std::move(ui)) {}
    
    void run() {
        // Coordinate components
    }
};
```

### Event System

```cpp
// Simple event system for component communication
class EventBus {
public:
    template<typename Event>
    void subscribe(std::function<void(const Event&)> handler);
    
    template<typename Event>
    void publish(const Event& event);
};

// Components use events for communication
class UIComponent {
    EventBus& events_;
public:
    void onButtonClick() {
        events_.publish(ProcessImageEvent{imagePath_});
    }
};
```

## 📋 Best Practices

### Component Design

1. **Single Responsibility** - Each component has one clear purpose
2. **Interface Segregation** - Small, focused interfaces
3. **Dependency Inversion** - Depend on abstractions, not concretions
4. **Open/Closed Principle** - Open for extension, closed for modification

### Performance Guidelines

```cpp
// Use RAII for resource management
class ComponentResource {
public:
    ComponentResource() { /* acquire resource */ }
    ~ComponentResource() { /* release resource */ }
    
    // No copy, move only
    ComponentResource(const ComponentResource&) = delete;
    ComponentResource(ComponentResource&&) = default;
};

// Prefer stack allocation
void processImage() {
    ImageProcessor processor;  // Stack allocated
    auto result = processor.process(image);
    // Automatic cleanup
}

// Use const-correctness
class ReadOnlyProcessor {
public:
    ProcessResult process(const Image& input) const;  // const method
private:
    const Settings settings_;  // const member
};
```

### Error Handling

```cpp
// Use exceptions for exceptional cases
class ComponentException : public std::runtime_error {
public:
    ComponentException(const std::string& message)
        : std::runtime_error("Component error: " + message) {}
};

// Return expected<T, Error> for normal error conditions
#include <expected>  // C++23
std::expected<ProcessResult, ErrorCode> process(const Input& input) {
    if (input.empty()) {
        return std::unexpected(ErrorCode::EMPTY_INPUT);
    }
    // Process and return result
    return ProcessResult{};
}
```

### Documentation

```cpp
/**
 * @brief Processes images using advanced algorithms
 * 
 * This component provides high-performance image processing
 * capabilities including filtering, enhancement, and format
 * conversion.
 * 
 * @example
 * ```cpp
 * ImageProcessor processor;
 * auto result = processor.process(inputImage);
 * if (result.success()) {
 *     auto outputImage = result.getImage();
 * }
 * ```
 */
class ImageProcessor {
public:
    /**
     * @brief Process an image with specified parameters
     * @param input The input image to process
     * @param params Processing parameters
     * @return ProcessResult containing output image or error
     */
    ProcessResult process(
        const Image& input, 
        const ProcessingParams& params = {}
    );
};
```

Your component development environment supports professional, scalable C++ architecture! 🎯