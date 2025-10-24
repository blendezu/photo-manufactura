# Build Instructions

## 📋 Table of Contents

1. [Requirements](#requirements)
2. [Quick Start](#quick-start)
3. [Build Configurations](#build-configurations)
4. [Component Development](#component-development)
5. [Testing](#testing)
6. [Advanced Options](#advanced-options)

## 🔧 Requirements

### System Requirements
- **CMake 3.21+**
- **C++20 compatible compiler**:
  - GCC 10+ 
  - Clang 12+
  - MSVC 2019+

### Dependencies
- **Qt6** (for UI components)
- **OpenCV 4.x** (for image processing)
- **LibRaw** (for RAW file support)
- **Ninja** (recommended build system)

### Installation (macOS)

```bash
# Install dependencies via Homebrew
brew install cmake ninja qt6 opencv

# Or install via package manager of choice
```

## 🚀 Quick Start

### Standard Build

```bash
# Clone repository
git clone https://github.com/ad-tran/photo-manufactura.git
cd photo-manufactura

# Configure and build
cmake --preset default
cmake --build --preset default

# Run
./build/default/bin/photo_manufactura
```

### Development Build

```bash
# Development build with extra warnings
cmake --preset dev
cmake --build --preset dev

# Run with debug info
./build/dev/bin/photo_manufactura
```

## ⚙️ Build Configurations

### Available Presets

| Preset | Description | Compiler Flags | Use Case |
|--------|-------------|----------------|----------|
| `default` | Standard release | `-O3` | Production |
| `debug` | Debug symbols | `-g -O0` | Debugging |
| `dev` | Debug + warnings | `-Wall -Wextra -Wpedantic` | Development |
| `release-optimized` | Maximum performance | `-O3 -march=native -flto` | Benchmarking |
| `macos-arm64` | Apple Silicon | `-arch arm64` | macOS specific |

### Manual Configuration

```bash
# List available presets
cmake --list-presets=all

# Configure with specific preset
cmake --preset debug

# Build with specific preset
cmake --build --preset debug

# Custom configuration
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 🧩 Component Development

### Independent Component Builds

Each component can be built and tested independently:

```bash
# UI Component
cd src/ui
cmake --preset debug
cmake --build --preset debug
./build/bin/test_mainwindow

# Controller Component  
cd src/controller
cmake --preset debug
cmake --build --preset debug
./build/bin/controller_test

# Image Processing Component
cd src/image_processing
cmake --preset debug
cmake --build --preset debug
./build/bin/image_test
```

### Component Architecture

```
src/
├── 🖼️  ui/                 # Qt6 user interface components
├── 🎮 controller/         # Application logic and coordination  
├── 📄 document/           # Document and project management
├── 🎨 image_processing/   # Core image manipulation algorithms
├── 📸 raw_processing/     # RAW file format handling
└── ⚡ scheduler_worker/   # Task scheduling and worker threads
```

### Integration Testing

```bash
# Build all components
cmake --build --preset all-components

# Build specific component from main project
cmake --build --preset ui-component
cmake --build --preset controller-component
```

## 🧪 Testing

### Running Tests

```bash
# All tests
ctest --preset default

# Component-specific tests
ctest --preset ui-tests
ctest --preset controller-tests

# Parallel test execution
ctest --preset parallel-tests

# Verbose test output
ctest --preset default --verbose
```

### Test Structure

```
tests/
├── unit/              # Unit tests per component
├── integration/       # Cross-component tests
└── performance/       # Benchmark tests
```

## 🔧 Advanced Options

### Custom CMake Variables

```bash
# Enable specific features
cmake --preset default -DENABLE_BENCHMARKS=ON
cmake --preset default -DENABLE_DOCS=ON
cmake --preset default -DBUILD_SHARED_LIBS=ON

# Custom installation prefix
cmake --preset default -DCMAKE_INSTALL_PREFIX=/usr/local

# Debug configuration
cmake --preset debug -DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -fsanitize=address"
```

### Cross-Platform Building

#### Windows (MSVC)
```bash
cmake --preset default -G "Visual Studio 16 2019"
cmake --build --preset default --config Release
```

#### Linux (GCC)
```bash
cmake --preset default -DCMAKE_CXX_COMPILER=g++-11
cmake --build --preset default
```

### Performance Optimization

```bash
# Maximum optimization
cmake --preset release-optimized

# Profile-guided optimization (advanced)
cmake --preset default -DCMAKE_CXX_FLAGS="-fprofile-generate"
# Run application to generate profile data
cmake --preset default -DCMAKE_CXX_FLAGS="-fprofile-use"
```

### Development Tools Integration

#### ccache (Compilation Caching)
```bash
# Install ccache
brew install ccache  # macOS
sudo apt install ccache  # Ubuntu

# Use with CMake
cmake --preset dev -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

#### Static Analysis
```bash
# Clang-tidy
cmake --preset dev -DCMAKE_CXX_CLANG_TIDY=clang-tidy

# Cppcheck
cmake --preset dev -DCMAKE_CXX_CPPCHECK=cppcheck
```

### Packaging and Distribution

```bash
# Create installation package
cmake --preset default
cmake --build --preset default
cmake --install build/default --prefix /usr/local

# Create distributable package
cpack --config build/default/CPackConfig.cmake
```

## 🐛 Troubleshooting

### Common Issues

- **Qt not found**: Ensure Qt6 is installed and `CMAKE_PREFIX_PATH` includes Qt
- **Missing dependencies**: Install all required packages via package manager
- **Compilation errors**: Check compiler version supports C++20
- **Linker errors**: Verify all dependencies are properly linked

### Getting Help

- Check [Troubleshooting Guide](troubleshooting.md)
- Review [VS Code Setup](vscode/setup.md) for IDE issues
- Check [Component Development](development/components.md) for architecture questions

For additional support, see the [Documentation Index](README.md) for comprehensive guides.