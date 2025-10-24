# 🔧 Troubleshooting Guide

## 📋 Table of Contents

1. [CMake Issues](#cmake-issues)
2. [Qt/MOC Problems](#qtmoc-problems)
3. [Component Build Failures](#component-build-failures)
4. [VS Code Integration](#vs-code-integration)
5. [Performance Issues](#performance-issues)
6. [Common Fixes](#common-fixes)

## 🏗️ CMake Issues

### Preset Not Found

**Problem**: `CMake Error: Could not read presets`

**Solutions**:
```bash
# Check preset file syntax
cmake --list-presets=all

# Validate JSON syntax
cat CMakePresets.json | jq .

# Reset and reconfigure
rm -rf build
cmake --preset debug
```

### Generator Issues

**Problem**: `CMake Error: Could not find generator Ninja`

**Solutions**:
```bash
# Install Ninja (macOS)
brew install ninja

# Use alternative generator
cmake -G "Unix Makefiles" -B build -S .

# Update preset to use available generator
```

### Cache Problems

**Problem**: `CMake Error: Cache mismatch`

**Solutions**:
```bash
# Clear cache and reconfigure
cmake --preset debug --fresh

# Manual cache clear
rm -rf build/CMakeCache.txt
rm -rf build/CMakeFiles/

# Full clean rebuild
rm -rf build && cmake --preset debug
```

## 🖼️ Qt/MOC Problems

### Undefined Symbols (Qt Classes)

**Problem**: 
```
Undefined symbols for architecture arm64:
"MainWindow::staticMetaObject", referenced from:
"vtable for MainWindow", referenced from:
```

**Root Cause**: Qt Meta-Object Compiler (MOC) not running

**Solutions**:

1. **Enable MOC in CMakeLists.txt**:
```cmake
# For standalone component builds
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()
```

2. **Verify Q_OBJECT macro**:
```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT  // Must be present for Qt slots/signals
public:
    MainWindow(QWidget* parent = nullptr);
};
```

3. **Check Qt package**:
```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
target_link_libraries(ui PUBLIC Qt6::Core Qt6::Widgets)
```

4. **Clean rebuild**:
```bash
rm -rf build
cmake --preset debug
cmake --build --preset debug
```

### Qt Not Found

**Problem**: `Could NOT find Qt6`

**Solutions**:
```bash
# Install Qt6 (macOS)
brew install qt6

# Set Qt path manually
export CMAKE_PREFIX_PATH="/opt/homebrew/lib/cmake/Qt6"

# Use specific Qt installation
cmake -DQt6_DIR="/path/to/qt6/lib/cmake/Qt6" --preset debug
```

## 🧩 Component Build Failures

### Target Name Mismatches

**Problem**: `Cannot specify sources for target "ui_test" which is not built`

**Root Cause**: Inconsistent target names in CMakeLists.txt

**Solution**: Ensure consistent naming:
```cmake
# ❌ Wrong: Different names
add_executable(test_mainwindow)
target_sources(ui_test PRIVATE ...)  # Wrong target name

# ✅ Correct: Consistent names
add_executable(test_mainwindow)
target_sources(test_mainwindow PRIVATE ...)
target_link_libraries(test_mainwindow PRIVATE ui)
```

### Missing Target Parameters

**Problem**: `target_link_libraries called with incorrect number of arguments`

**Solution**: Always specify target name:
```cmake
# ❌ Wrong: Missing target name
target_link_libraries( PRIVATE ui)

# ✅ Correct: Include target name
target_link_libraries(test_mainwindow PRIVATE ui)
```

### Conditional Build Logic

**Problem**: Targets not available in different build modes

**Solution**: Proper conditional logic:
```cmake
# Test executable only in standalone mode
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(test_component)
    target_sources(test_component PRIVATE test_main.cpp)
    target_link_libraries(test_component PRIVATE component_name)
endif()
```

## 💻 VS Code Integration

### CMake Extension Not Working

**Problem**: CMake extension doesn't detect presets

**Solutions**:
1. **Install required extensions**:
   - CMake Tools (`ms-vscode.cmake-tools`)
   - C/C++ (`ms-vscode.cpptools`)

2. **Check settings**:
```json
{
    "cmake.useCMakePresets": "always",
    "cmake.configureOnOpen": true
}
```

3. **Reset CMake**:
   - `Cmd+Shift+P` → `CMake: Reset Cache and Configure`
   - Reload VS Code window

### IntelliSense Issues

**Problem**: No code completion or error highlighting

**Solutions**:
1. **Ensure compile commands**:
```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

2. **Check configuration provider**:
```json
{
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

3. **Verify build directory**:
   - Look for `compile_commands.json` in build folder
   - Ensure VS Code is using correct build directory

### Debugging Problems

**Problem**: Debugger can't find executable

**Solutions**:
1. **Check launch.json paths**:
```json
{
    "program": "${workspaceFolder}/src/ui/build/bin/test_mainwindow",
    "cwd": "${workspaceFolder}/src/ui"
}
```

2. **Verify executable exists**:
```bash
ls -la src/ui/build/bin/test_mainwindow
```

3. **Build before debugging**:
   - Set `"preLaunchTask": "Build UI Component"`

## ⚡ Performance Issues

### Slow Builds

**Solutions**:
```cmake
# Use Ninja generator
set(CMAKE_GENERATOR "Ninja")

# Parallel builds
cmake --build . -j$(nproc)

# Precompiled headers
target_precompile_headers(target_name PRIVATE pch.h)
```

### Memory Issues

**Solutions**:
```bash
# Reduce parallel jobs
cmake --build . -j4

# Check memory usage
top -pid $(pgrep cmake)

# Clean unnecessary files
rm -rf build/*/CMakeFiles/
```

## 🔧 Common Fixes

### Clean Everything

```bash
# Complete project clean
rm -rf build/
rm -rf src/*/build/
rm -rf .vscode/.cache/

# Reconfigure from scratch
cmake --preset debug
cmake --build --preset debug
```

### Reset VS Code

```bash
# Reset CMake extension
Cmd+Shift+P → CMake: Reset Cache and Configure

# Reload window
Cmd+Shift+P → Developer: Reload Window

# Clear workspace state
rm -rf .vscode/settings.json.bak
```

### Update Configuration

```bash
# Update CMake presets
cmake --list-presets=all  # Verify syntax

# Validate JSON files
cat CMakePresets.json | jq .
cat .vscode/settings.json | jq .

# Check file permissions
chmod +x src/*/build/bin/*
```

### Dependency Issues

```bash
# Update package manager
brew update && brew upgrade

# Reinstall Qt6
brew uninstall qt6 && brew install qt6

# Check CMake version
cmake --version  # Should be 3.21+

# Verify compiler
clang++ --version
```

## 📋 Quick Diagnostic Commands

```bash
# System info
cmake --version
clang++ --version
ninja --version

# Project status
cmake --list-presets=all
cmake --build . --target help

# Component status
cd src/ui
cmake --preset debug 2>&1 | head -20
ls -la build/bin/

# VS Code status
code --list-extensions | grep cmake
cat .vscode/settings.json | jq .cmake
```

## 🆘 Emergency Reset

If everything breaks:

```bash
# 1. Clean everything
rm -rf build/ src/*/build/ .vscode/.cache/

# 2. Reset git (if needed)
git clean -fdx
git reset --hard HEAD

# 3. Restart VS Code
# Close VS Code completely, then reopen

# 4. Reconfigure
cmake --preset debug
cmake --build --preset debug

# 5. Test component
cd src/ui
cmake --preset debug
./build/bin/test_mainwindow
```

Your troubleshooting toolkit is now comprehensive and battle-tested! 🛠️