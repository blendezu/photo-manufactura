# 🏗️ CMake Development Guide

## 📋 Table of Contents

1. [Project Structure](#project-structure)
2. [Build System Overview](#build-system-overview)
3. [CMake Presets](#cmake-presets)
4. [Component Development](#component-development)
5. [Build Configurations](#build-configurations)
6. [Testing](#testing)
7. [Troubleshooting](#troubleshooting)

## 🏛️ Project Structure

```
photo-manufactura/
├── 📄 CMakeLists.txt              # Main build configuration
├── 🔧 CMakePresets.json           # Build presets
├── 📦 src/                        # Source code
│   ├── 🖥️  controller/             # Application logic
│   │   ├── CMakeLists.txt         # Component build config
│   │   └── CMakePresets.json      # Component presets
│   ├── 🖼️  ui/                     # Qt6 user interface
│   │   ├── CMakeLists.txt         # UI build config
│   │   ├── CMakePresets.json      # UI presets
│   │   ├── mainwindow.cpp/.h      # Qt main window
│   │   └── test_mainwindow.cpp    # UI tests
│   ├── 🎨 image_processing/       # Image algorithms
│   ├── 📸 raw_processing/         # RAW file handling
│   └── ⚡ scheduler_worker/       # Task scheduling
└── 🔨 build/                      # Build outputs
    ├── default/                   # Release builds
    ├── debug/                     # Debug builds
    └── dev/                       # Development builds
```

## 🎯 Build System Overview

### Architecture Principles

- **🧩 Modular**: Each component builds independently
- **🔄 Reusable**: Components can be libraries or executables
- **⚡ Fast**: Parallel builds and incremental compilation
- **🎨 Flexible**: Multiple build configurations via presets

### Component Independence

Each component supports:
- ✅ **Standalone builds** - Build and test without main project
- ✅ **Integration builds** - Link into main application
- ✅ **Independent testing** - Component-specific test executables
- ✅ **Separate development** - Different teams can work independently

## 🎛️ CMake Presets

### Main Project Presets

| Preset | Description | Compiler Flags | Use Case |
|--------|-------------|----------------|----------|
| `default` | Standard release | `-O3` | Production deployment |
| `debug` | Debug symbols | `-g -O0` | General debugging |
| `dev` | Development warnings | `-Wall -Wextra -Wpedantic` | Active development |
| `release-optimized` | Maximum performance | `-O3 -march=native -flto` | Performance testing |
| `macos-arm64` | Apple Silicon | `-arch arm64` | macOS specific |

### Component Presets

Each component folder has independent presets:

```bash
# UI Component presets
cd src/ui
cmake --list-presets
# Available: debug, release, ui-lib-only, ui-test-only
```

## 🚀 Component Development

### Standalone Component Build

```bash
# Build UI component independently
cd src/ui
cmake --preset debug                 # Configure
cmake --build --preset debug         # Build everything
cmake --build --preset ui-lib-only   # Build just library
./build/bin/test_mainwindow          # Run tests
```

### Integrated Project Build

```bash
# Build full project
cmake --preset dev                    # Configure with warnings
cmake --build --preset dev           # Build all components
cmake --build --preset ui-component  # Build UI only
./build/dev/bin/photo_manufactura    # Run main app
```

### Component CMakeLists.txt Pattern

```cmake
# Standalone mode detection
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.21)
    project(component_name VERSION 1.0.0 LANGUAGES CXX)
    
    # Component-specific settings
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    
    # Qt components need MOC
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# Create library
add_library(component_name STATIC)
target_sources(component_name PRIVATE source.cpp source.h)
target_include_directories(component_name PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Dependencies
find_package(Qt6 COMPONENTS Core Widgets)
if(Qt6_FOUND)
    target_link_libraries(component_name PUBLIC Qt6::Core Qt6::Widgets)
endif()

# Test executable (standalone only)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(test_component)
    target_sources(test_component PRIVATE test_main.cpp)
    target_link_libraries(test_component PRIVATE component_name)
endif()
```

## ⚙️ Build Configurations

### Development Workflow

```bash
# 1. Development Cycle
cmake --preset dev                    # Configure with warnings
cmake --build --preset dev           # Build with debug info
ctest --preset debug                  # Run tests
./build/dev/bin/photo_manufactura    # Test application

# 2. Component Development
cd src/ui
cmake --preset debug                 # Configure component
cmake --build --preset debug         # Build and test
./build/bin/test_mainwindow          # Component testing

# 3. Performance Testing
cmake --preset release-optimized     # Maximum optimization
cmake --build --preset release-optimized
# Run performance benchmarks...

# 4. Production Release
cmake --preset default               # Standard release
cmake --build --preset default
# Package for distribution...
```

### Build Targets

```bash
# Build specific components
cmake --build --preset ui-component
cmake --build --preset controller-component
cmake --build --preset all-components

# Build types
cmake --build --preset clean-build   # Clean first
cmake --build --preset debug         # Debug version
cmake --build --preset default       # Release version
```

## 🧪 Testing

### Test Organization

```bash
# Component-level tests
cd src/ui && ./build/bin/test_mainwindow
cd src/controller && ./build/bin/controller_test

# Project-level tests
ctest --preset default               # All tests
ctest --preset ui-tests             # UI component tests only
ctest --preset controller-tests     # Controller tests only
ctest --preset parallel-tests       # Parallel execution
```

### Test Development

```cmake
# In component CMakeLists.txt
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    enable_testing()
    
    add_executable(test_component test_main.cpp)
    target_link_libraries(test_component PRIVATE component_name)
    
    add_test(NAME component_basic_test COMMAND test_component)
endif()
```

## 🔧 Troubleshooting

### Common Issues

#### Build Failures
```bash
# Clean and reconfigure
cmake --preset dev --fresh           # Force reconfigure
cmake --build --preset dev --clean-first  # Clean build
```

#### Qt MOC Problems
```bash
# Check Qt setup
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
set(CMAKE_AUTOMOC ON)  # Enable Meta-Object Compiler

# Verify Q_OBJECT macro in classes
class MyWidget : public QWidget {
    Q_OBJECT  // Required for Qt slots/signals
    // ...
};
```

#### Preset Issues
```bash
# Debug presets
cmake --list-presets=all             # Show all available
cmake --preset debug 2>&1 | head    # Show configuration errors
```

#### Component Linking
```bash
# Check target dependencies
cmake --build . --target help        # List all targets
cmake --build . --target component_name --verbose  # Verbose build
```

### Performance Optimization

```cmake
# Release optimizations
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

# Debug optimizations  
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer")

# Link-time optimization
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
```

### Memory and Debugging

```bash
# Memory debugging (requires tools)
cmake --preset dev -DCMAKE_CXX_FLAGS="-fsanitize=address"

# Thread debugging
cmake --preset dev -DCMAKE_CXX_FLAGS="-fsanitize=thread"

# Static analysis
cmake --preset dev -DCMAKE_CXX_CLANG_TIDY=clang-tidy
```

## 📊 Build Performance

### Optimization Tips

1. **Parallel Builds**: Use `-j$(nproc)` or set in presets
2. **Ninja Generator**: Faster than Make for large projects
3. **Unity Builds**: Combine source files to reduce compile time
4. **Precompiled Headers**: Speed up template-heavy code
5. **ccache**: Cache compilation results

### Build Time Comparison

| Configuration | Build Time | Use Case |
|---------------|------------|----------|
| `debug` | ~30s | Development |
| `dev` | ~45s | Code review |
| `default` | ~60s | Release testing |
| `release-optimized` | ~90s | Performance builds |

Your CMake build system is now optimized for both development speed and production quality! 🎯