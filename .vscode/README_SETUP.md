# VS Code CMake Extension Setup Guide

## 🎯 Quick Start

Your VS Code workspace is now configured for independent component development!

## 🚀 How to Use

### Method 1: CMake Extension UI
1. **Open Command Palette**: `Cmd+Shift+P` (macOS) or `Ctrl+Shift+P` (Windows/Linux)
2. **Select Commands**:
   - `CMake: Select Configure Preset` - Choose build type (default, debug, dev)
   - `CMake: Configure` - Configure current folder
   - `CMake: Build` - Build current folder
   - `CMake: Debug` - Debug current target

### Method 2: VS Code Tasks (Recommended)
1. **Open Command Palette**: `Cmd+Shift+P`
2. **Run Task**: Type `Tasks: Run Task`
3. **Available Tasks**:
   - `Build UI Component` - Build just the UI
   - `Build Controller Component` - Build just the controller
   - `Build All Components` - Build all components in parallel
   - `Test UI Component` - Run UI tests

### Method 3: Multi-Root Workspace
1. **Open Workspace**: `File → Open Workspace from File`
2. **Select**: `photo-manufactura.code-workspace`
3. **Switch Folders**: Click folder name in Explorer sidebar
4. **Build**: Each folder becomes an independent project

### Method 4: Keyboard Shortcuts
- `Cmd+Shift+B` - Build current project
- `Cmd+Shift+C` - Configure CMake
- `Cmd+Shift+R` - Debug target
- `Cmd+Shift+T` - Select target
- `Cmd+Shift+U` - Build UI Component (custom)
- `Cmd+Shift+I` - Build Controller Component (custom)

## 🔧 Status Bar Usage

Look at the bottom status bar for CMake controls:
1. **Kit Selection** (compiler) - Click to change
2. **Configure Preset** - Click to change build type
3. **Build Target** - Click to select what to build
4. **Build Button** ⚙️ - Click to build
5. **Debug Button** 🐛 - Click to debug

## 📁 Component Development Workflow

### Building UI Component Independently:
```bash
# Option 1: Via VS Code Tasks
Cmd+Shift+P → Tasks: Run Task → Build UI Component

# Option 2: Via Terminal
cd src/ui
cmake -B build -S .
cmake --build build
./build/bin/ui_test
```

### Debugging Components:
1. **Set Breakpoints** in component files
2. **Select Debug Config**: "Debug UI Component"
3. **Press F5** or click debug button
4. **Debug Session** starts with just that component

## 🎨 Component Folders

Each component folder (`src/ui/`, `src/controller/`, etc.) can be:
- ✅ **Configured independently** with CMake
- ✅ **Built independently** 
- ✅ **Tested independently**
- ✅ **Debugged independently**
- ✅ **Developed by different team members**

## 🔄 Integration Testing

After component development:
```bash
# Return to main project
cd ../..

# Build full project
cmake --preset default
cmake --build --preset default

# Run integration tests
./build/default/bin/photo_manufactura
```

## 🛠️ Troubleshooting

### CMake Extension Not Working?
1. Install: `ms-vscode.cmake-tools`
2. Reload VS Code
3. Open Command Palette → `CMake: Reset Cache and Configure`

### Component Build Failing?
1. Check `src/[component]/CMakeLists.txt` exists
2. Ensure component has source files
3. Run: Tasks → Configure [Component] Component

### IntelliSense Issues?
1. Ensure `compile_commands.json` exists in build folder
2. Check C++ extension is installed
3. Reload VS Code

Enjoy your modular development workflow! 🚀