# 🎯 CMake Presets Guide for Photo Manufactura

## 📋 **Available Presets**

### 🏗️ **Main Project Presets** (`/CMakePresets.json`)

| Preset Name | Description | Use Case |
|-------------|-------------|----------|
| `default` | Standard release build | Production deployment |
| `debug` | Debug with symbols | General development |
| `dev` | Debug + extra warnings | Active development |
| `release-optimized` | Highly optimized | Performance testing |
| `macos-arm64` | macOS ARM64 specific | Apple Silicon builds |
| `component-debug` | Component development | Standalone components |
| `component-release` | Component release | Component testing |

### 🧩 **Component-Specific Presets**

Each component folder (`src/ui/`, `src/controller/`, etc.) has its own `CMakePresets.json`:

| Component | Available Presets | Purpose |
|-----------|------------------|---------|
| **UI** | `debug`, `release` | Standalone UI development |
| **Controller** | `debug`, `release` | Controller logic testing |
| **Image Processing** | `debug`, `release` | Algorithm development |

## 🚀 **How to Use Presets**

### **Method 1: Command Line**

```bash
# Main project
cmake --preset dev                    # Configure with dev preset
cmake --build --preset dev           # Build with dev preset
ctest --preset debug                  # Test with debug preset

# Component (from component folder)
cd src/ui
cmake --preset debug                 # Configure UI component
cmake --build --preset debug         # Build UI component
```

### **Method 2: VS Code CMake Tools**

1. **Status Bar**: Click preset name in status bar
2. **Command Palette**: `Cmd+Shift+P` → `CMake: Select Configure Preset`
3. **Quick Pick**: Choose from available presets

### **Method 3: VS Code Tasks**

Available in Tasks menu (`Cmd+Shift+P` → `Tasks: Run Task`):
- Build UI Component
- Build Controller Component  
- Build All Components

## 🔧 **Preset Configuration Details**

### **Configure Presets**

```json
{
    "name": "dev",
    "displayName": "Development",
    "description": "Debug build with extra warnings for development",
    "inherits": ["debug"],
    "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_CXX_FLAGS": "-Wall -Wextra -Wpedantic -Wconversion -Wshadow"
    }
}
```

### **Build Presets**

```json
{
    "name": "ui-component",
    "displayName": "UI Component Only",
    "configurePreset": "default",
    "targets": ["ui"],
    "jobs": 4
}
```

### **Test Presets**

```json
{
    "name": "ui-tests",
    "displayName": "UI Component Tests",
    "configurePreset": "default",
    "filter": {
        "include": {
            "name": "ui.*"
        }
    }
}
```

## 🎯 **Development Workflows**

### **Full Project Development**

```bash
# Development cycle
cmake --preset dev                    # Configure with warnings
cmake --build --preset dev           # Build with debug info
ctest --preset debug                  # Run all tests
./build/dev/bin/photo_manufactura    # Run application
```

### **Component Development** 

```bash
# UI Component workflow
cd src/ui
cmake --preset debug                 # Configure component
cmake --build --preset ui-lib-only   # Build just the library
cmake --build --preset ui-test-only  # Build just tests
./build/bin/ui_test                   # Run component tests
```

### **Performance Testing**

```bash
# Optimized builds
cmake --preset release-optimized     # Maximum optimization
cmake --build --preset release-optimized
# Performance profiling...
```

### **Cross-Platform Development**

```bash
# macOS ARM64 specific
cmake --preset macos-arm64           # Apple Silicon optimized
cmake --build --preset default
```

## ⚙️ **VS Code Integration**

### **Status Bar Controls**

Look for these controls in your VS Code status bar:

| Icon | Function | Click Action |
|------|----------|-------------|
| 🔧 | Configure Preset | Select configure preset |
| 🏗️ | Build Preset | Select build preset |
| 🎯 | Target | Select build target |
| ▶️ | Build | Start build |
| 🐛 | Debug | Start debugging |

### **Command Palette Commands**

- `CMake: Select Configure Preset`
- `CMake: Select Build Preset`  
- `CMake: Select Test Preset`
- `CMake: Build`
- `CMake: Test`
- `CMake: Debug`

### **Keyboard Shortcuts**

| Shortcut | Action |
|----------|--------|
| `Cmd+Shift+P` | Configure preset |
| `Cmd+Shift+B` | Build |
| `Cmd+Shift+T` | Select target |
| `Cmd+Shift+R` | Debug |

## 🔍 **Troubleshooting**

### **Preset Not Found**
```bash
# List available presets
cmake --list-presets=all

# Check preset file syntax
cmake --preset nonexistent  # Will show available presets
```

### **VS Code Not Showing Presets**
1. Reload VS Code window
2. `CMake: Reset Cache and Configure`
3. Check `.vscode/settings.json` has `"cmake.useCMakePresets": "always"`

### **Component Preset Issues**
1. Ensure component `CMakePresets.json` exists
2. Check component `CMakeLists.txt` is configured for standalone build
3. Verify current working directory when running commands

### **Build Failures**
```bash
# Clean and reconfigure
cmake --preset dev --fresh           # Force reconfigure
cmake --build --preset dev --clean-first  # Clean build
```

## 📊 **Preset Performance Comparison**

| Preset | Build Time | Binary Size | Debug Info | Warnings |
|--------|------------|-------------|------------|----------|
| `debug` | Fast | Large | Full | Basic |
| `dev` | Medium | Large | Full | Extended |
| `default` | Medium | Small | None | Basic |
| `release-optimized` | Slow | Smallest | None | Basic |

## 🎪 **Advanced Usage**

### **Custom Environment Variables**

Add to your presets:
```json
"environment": {
    "QT_QPA_PLATFORM": "offscreen",
    "DISPLAY": ":0"
}
```

### **Conditional Presets**

```json
"condition": {
    "type": "equals",
    "lhs": "${hostSystemName}",
    "rhs": "Darwin"
}
```

Your CMake presets are now optimized for both main project and component-level development! 🚀