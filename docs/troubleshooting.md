# Troubleshooting Guide

Solutions for common Photo Manufactura issues.

## CMake Issues

### Preset Not Found
```bash
cmake --list-presets=all  # Check available
cat CMakePresets.json | jq .  # Validate JSON
rm -rf build && cmake --preset debug  # Reset
```

### Cache Problems
```bash
cmake --preset debug --fresh  # Force reconfigure
rm -rf build/CMakeCache.txt build/CMakeFiles/  # Manual clear
```

### Generator Not Found
```bash
brew install ninja  # Install Ninja
cmake -G "Unix Makefiles" -B build -S .  # Alternative
```

## Qt/MOC Issues

### Undefined Symbols
```
Undefined symbols: "MainWindow::staticMetaObject"
```

**Fix**: Enable MOC in CMakeLists.txt:
```cmake
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
```

Verify `Q_OBJECT` macro in header:
```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT  // Required!
};
```

### Qt Not Found
```bash
brew install qt6
export CMAKE_PREFIX_PATH="/opt/homebrew/lib/cmake/Qt6"
```

## Component Errors

### Target Mismatch
```cmake
# Wrong
add_executable(test_main)
target_sources(ui_test PRIVATE ...)  # Different name!

# Correct
add_executable(test_main)
target_sources(test_main PRIVATE ...)
```

## VS Code Issues

### CMake Extension
1. Install `ms-vscode.cmake-tools`
2. `Cmd+Shift+P` → `Developer: Reload Window`
3. `Cmd+Shift+P` → `CMake: Reset Cache and Configure`

### IntelliSense
- Check `compile_commands.json` exists in build/
- Set `"C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"`

## Performance

### Slow Builds
```bash
cmake --build . -j$(nproc)  # Parallel
brew install ccache  # Caching
```

## Emergency Reset

```bash
# 1. Clean everything
rm -rf build/ src/*/build/ .vscode/.cache/

# 2. Reconfigure
cmake --preset debug
cmake --build --preset debug

# 3. Test
./build/debug/bin/photo_manufactura
```

## Diagnostics

```bash
cmake --version
clang++ --version
ninja --version
cmake --list-presets=all
```
