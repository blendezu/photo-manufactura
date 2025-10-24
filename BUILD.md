# Photo Manufactura

A modular C++ application for photo processing and manufacturing workflows.

## Build Requirements

- CMake 3.21+
- C++20 compatible compiler
- Qt6 (optional, for UI components)
- OpenCV (optional, for image processing)

## Quick Start

```bash
# Configure
cmake --preset default

# Build
cmake --build --preset default

# Run
./build/default/bin/photo_manufactura
```

## Development

### Component Development
Each component can be developed independently:

```bash
cd src/ui
cmake -B build -S .
cmake --build build
./build/bin/ui_test
```

### Available Presets
- `default` - Release build
- `debug` - Debug build with symbols
- `dev` - Debug with extra warnings
- `release-optimized` - Highly optimized release

## Architecture

- `src/ui/` - User interface components
- `src/controller/` - Application logic
- `src/image_processing/` - Image manipulation
- `src/raw_processing/` - RAW file handling
- `src/scheduler_worker/` - Task scheduling

## Testing

```bash
cmake --build --preset default
ctest --preset default
```