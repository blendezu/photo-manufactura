# 🛠️ VS Code Development Setup

## 📋 Table of Contents

1. [Overview](#overview)
2. [Extensions](#extensions)
3. [Configuration](#configuration)
4. [CMake Integration](#cmake-integration)
5. [Component Development](#component-development)
6. [Debugging](#debugging)
7. [Troubleshooting](#troubleshooting)

## 🎯 Overview

This guide sets up VS Code for professional C++ development with CMake, Qt6, and component-based architecture for the Photo Manufactura project.

## 🔌 Required Extensions

Install these extensions (auto-recommended in `.vscode/extensions.json`):

- **CMake Tools** (`ms-vscode.cmake-tools`) - CMake integration
- **C/C++** (`ms-vscode.cpptools`) - IntelliSense and debugging
- **CMake** (`twxs.cmake`) - CMake language support
- **clangd** (`llvm-vs-code-extensions.vscode-clangd`) - Advanced C++ language server

## ⚙️ Configuration

All configuration is automated through:

### 📁 Configuration Files
```
.vscode/
├── settings.json         # VS Code and extension settings
├── tasks.json           # Build and test tasks
├── launch.json          # Debug configurations
├── cmake-kits.json      # Compiler configurations
├── keybindings.json     # Custom keyboard shortcuts
└── extensions.json      # Recommended extensions
```

### 🔧 Key Settings

```json
{
    "cmake.useCMakePresets": "always",
    "cmake.configureOnOpen": true,
    "cmake.generator": "Ninja",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

## 🏗️ CMake Integration

### Status Bar Controls

| Icon | Function | Description |
|------|----------|-------------|
| 🔧 | Configure Preset | Select build configuration |
| 🎯 | Build Target | Choose what to build |
| ▶️ | Build | Start compilation |
| 🐛 | Debug | Launch debugger |

### Available Presets

| Preset | Purpose | Optimization |
|--------|---------|-------------|
| `default` | Production release | `-O3` |
| `debug` | Development | `-g -O0` |
| `dev` | Development + warnings | `-Wall -Wextra` |
| `release-optimized` | Maximum performance | `-O3 -march=native` |

### Quick Commands

- **Configure**: `Cmd+Shift+P` → `CMake: Configure`
- **Build**: `Cmd+Shift+P` → `CMake: Build`
- **Select Target**: `Cmd+Shift+P` → `CMake: Select Target`

## 🧩 Component Development

### Multi-Root Workspace

Open `photo-manufactura.code-workspace` for component-focused development:

```
📁 Main Project          # Full project view
📁 UI Component          # src/ui/ as standalone project
📁 Controller Component  # src/controller/ as standalone
📁 Image Processing      # src/image_processing/ as standalone
```

### Component Workflow

```bash
# Method 1: VS Code Tasks
Cmd+Shift+P → Tasks: Run Task → Build UI Component

# Method 2: Status Bar
Click folder → Select src/ui → Click build button

# Method 3: Command Line
cd src/ui
cmake --preset debug
cmake --build --preset debug
```

### Independent Component Testing

Each component can be tested standalone:

```bash
cd src/ui
./build/bin/test_mainwindow    # UI component test
```

## 🐛 Debugging

### Available Debug Configurations

- **Debug Main App (Default)** - Full application debugging
- **Debug Main App (Dev)** - Development build debugging  
- **Debug UI Component** - Standalone UI debugging
- **Debug Controller Component** - Standalone controller debugging

### Debugging Workflow

1. **Set Breakpoints** in source files
2. **Select Configuration** from debug dropdown
3. **Press F5** or click debug button
4. **Step Through Code** with standard debug controls

### Component-Specific Debugging

Debug individual components without building the full project:

```bash
# Automatically builds and debugs UI component
F5 → Debug UI Component
```

## 🔍 Troubleshooting

### CMake Extension Not Working

1. **Install CMake Tools**: `ms-vscode.cmake-tools`
2. **Reload Window**: `Cmd+Shift+P` → `Developer: Reload Window`
3. **Reset CMake**: `Cmd+Shift+P` → `CMake: Reset Cache and Configure`

### Component Build Failures

```bash
# Clean component build
cd src/ui
rm -rf build
cmake --preset debug
cmake --build --preset debug
```

### Qt MOC Issues

If you see undefined symbols with Qt classes:

1. Ensure `CMAKE_AUTOMOC ON` in component CMakeLists.txt
2. Verify Qt classes have `Q_OBJECT` macro
3. Check `find_package(Qt6 COMPONENTS ...)` is called
4. Clean rebuild: `rm -rf build && cmake --preset debug`

### IntelliSense Problems

1. Ensure `compile_commands.json` exists in build folder
2. Check C++ extension is active
3. Verify clangd path in settings
4. Reload VS Code window

### Preset Issues

```bash
# List available presets
cmake --list-presets=all

# Check preset file syntax
cmake --preset debug  # Will show errors if invalid
```

## ⌨️ Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Cmd+Shift+B` | Build current project |
| `Cmd+Shift+C` | Configure CMake |
| `Cmd+Shift+R` | Debug target |
| `Cmd+Shift+T` | Select target |
| `Cmd+Shift+U` | Build UI Component |
| `Cmd+Shift+I` | Build Controller Component |

## 🚀 Quick Start Checklist

- [ ] Open VS Code in project root
- [ ] Install recommended extensions (auto-prompted)
- [ ] Select CMake kit (Clang recommended)
- [ ] Choose configure preset (`dev` for development)
- [ ] Build project (`Cmd+Shift+B`)
- [ ] Test component builds (`Tasks → Build UI Component`)
- [ ] Set up debugging (`F5`)

Your VS Code environment is now optimized for professional C++ component development! 🎯