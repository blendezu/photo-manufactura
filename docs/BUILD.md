# Build Instructions

Quick build guide for Photo Manufactura.

## Requirements

| Requirement | Version | Installation (macOS) |
|-------------|---------|---------------------|
| CMake | 3.21+ | `brew install cmake` |
| C++ Compiler | C++20 (GCC 10+, Clang 12+) | Xcode CLI Tools |
| Qt6 | 6.x | `brew install qt6` |
| OpenCV | 4.x | `brew install opencv` |
| LibRaw | 0.20+ | `brew install libraw` |
| Ninja | latest | `brew install ninja` |

## Quick Start

```bash
# Clone & build
git clone https://github.com/ad-tran/photo-manufactura.git
cd photo-manufactura
cmake --preset default && cmake --build --preset default

# Run
./build/default/bin/photo_manufactura
```

## Build Presets

| Preset | Use Case |
|--------|----------|
| `default` | Production build |
| `debug` | Debugging with symbols |
| `dev` | Development (fast builds) |
| `release-optimized` | Performance testing |

```bash
# List all presets
cmake --list-presets=all

# Build with preset
cmake --preset dev && cmake --build --preset dev
```

> **Advanced CMake**: See [CMake Development Guide](development/cmake.md) for optimization flags, sanitizers, and testing configuration.

## Component Builds

Build components independently:

```bash
cd src/ui
cmake --preset debug && cmake --build --preset debug
./build/bin/test_mainwindow
```

## Cross-Platform

### Windows (MSVC)
```bash
cmake --preset default -G "Visual Studio 16 2019"
cmake --build --preset default --config Release
```

### Linux (GCC)
```bash
cmake --preset default -DCMAKE_CXX_COMPILER=g++-11
cmake --build --preset default
```

## Related Guides

- [CMake Development](development/cmake.md) - Advanced CMake usage
- [Component Development](development/components.md) - Architecture guide
- [Troubleshooting](troubleshooting.md) - Build issues
