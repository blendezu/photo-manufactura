# 📚 Documentation Index

Welcome to the Photo Manufactura development documentation!

## 🚀 Quick Start

- **[README](../README.md)** - Project overview and quick start
- **[Build Instructions](BUILD.md)** - Detailed build setup and requirements
- **[VS Code Setup](vscode/setup.md)** - Complete IDE configuration

## 🛠️ Development

### Core Guides
- **[CMake Development](development/cmake.md)** - Build system and presets
- **[Component Development](development/components.md)** - Modular architecture guide
- **[Troubleshooting](troubleshooting.md)** - Common issues and solutions

### VS Code Integration
- **[VS Code Setup](vscode/setup.md)** - IDE configuration and workflow
- **[Extension Configuration](../.vscode/)** - Automated VS Code settings

## 🏗️ Architecture

### Project Structure
```
photo-manufactura/
├── 📄 README.md                    # Project overview
├── 📖 docs/                        # Documentation
│   ├── BUILD.md                    # Build instructions
│   ├── troubleshooting.md          # Problem solving
│   ├── development/                # Development guides
│   │   ├── cmake.md               # CMake usage
│   │   └── components.md          # Component development
│   └── vscode/                     # VS Code configuration
│       └── setup.md               # IDE setup guide
├── 🔧 CMakeLists.txt               # Main build configuration
├── ⚙️  CMakePresets.json            # Build presets
├── 🛠️  .vscode/                     # VS Code configuration
│   ├── settings.json              # IDE settings
│   ├── tasks.json                 # Build tasks
│   ├── launch.json                # Debug configurations
│   └── ...                        # Other VS Code configs
└── 📦 src/                         # Source code
    ├── 🖼️  ui/                      # Qt6 user interface
    ├── 🎮 controller/              # Application logic
    ├── 🎨 image_processing/        # Image algorithms
    ├── 📸 raw_processing/          # RAW file handling
    └── ⚡ scheduler_worker/        # Task scheduling
```

### Component Architecture

Each component (`src/*/`) is independently buildable:

- **CMakeLists.txt** - Component build configuration
- **CMakePresets.json** - Component-specific presets
- **Source files** - Implementation (.cpp/.h)
- **Test files** - Component tests
- **build/** - Build artifacts (generated)

## 📋 Documentation Categories

### 🎯 **Getting Started**
1. [Project Overview](../README.md#-photo-manufactura)
2. [Prerequisites](BUILD.md#prerequisites) 
3. [Quick Build](../README.md#-quick-start)
4. [VS Code Setup](vscode/setup.md#-quick-start-checklist)

### 🔧 **Development**
1. [CMake Presets](development/cmake.md#%EF%B8%8F-cmake-presets)
2. [Component Creation](development/components.md#-development-workflow)
3. [Testing Strategy](development/components.md#-testing-strategy)
4. [Performance Tips](development/cmake.md#-build-performance)

### 🐛 **Debugging**
1. [Build Failures](troubleshooting.md#%EF%B8%8F-cmake-issues)
2. [Qt/MOC Issues](troubleshooting.md#%EF%B8%8F-qtmoc-problems)
3. [Component Problems](troubleshooting.md#-component-build-failures)
4. [VS Code Issues](troubleshooting.md#-vs-code-integration)

### ⚙️ **Configuration**
1. [CMake Presets Reference](development/cmake.md#-cmake-presets)
2. [VS Code Settings](vscode/setup.md#%EF%B8%8F-configuration)
3. [Build Configurations](development/cmake.md#%EF%B8%8F-build-configurations)
4. [Component Templates](development/components.md#-component-structure)

## 🔗 Quick Reference

### Common Commands

```bash
# Build entire project
cmake --preset dev && cmake --build --preset dev

# Build component independently
cd src/ui && cmake --preset debug && cmake --build --preset debug

# Run tests
./build/dev/bin/photo_manufactura          # Main app
./src/ui/build/bin/test_mainwindow         # UI component test

# Clean builds
rm -rf build && cmake --preset debug       # Clean main project
cd src/ui && rm -rf build                  # Clean component
```

### VS Code Shortcuts

| Shortcut | Action |
|----------|--------|
| `Cmd+Shift+B` | Build current project |
| `Cmd+Shift+P` → `CMake: Configure` | Configure CMake |
| `Cmd+Shift+P` → `Tasks: Run Task` | Run component tasks |
| `F5` | Debug current configuration |

### Preset Quick Reference

| Preset | Purpose | Use Case |
|--------|---------|----------|
| `default` | Release build | Production |
| `debug` | Debug symbols | Development |
| `dev` | Debug + warnings | Active coding |
| `release-optimized` | Maximum performance | Benchmarking |

## 🆘 Emergency Procedures

### Complete Reset
```bash
# If everything breaks
rm -rf build/ src/*/build/ .vscode/.cache/
cmake --preset debug
cmake --build --preset debug
```

### Quick Fixes
- **CMake issues**: [troubleshooting.md#cmake-issues](troubleshooting.md#%EF%B8%8F-cmake-issues)
- **Qt problems**: [troubleshooting.md#qt-moc-problems](troubleshooting.md#%EF%B8%8F-qtmoc-problems)
- **VS Code setup**: [vscode/setup.md#troubleshooting](vscode/setup.md#-troubleshooting)

## 📞 Support

- **🐛 Bug Reports**: Use [GitHub Issues](../.github/ISSUE_TEMPLATE/bug_report.md)
- **💡 Feature Requests**: Use [GitHub Issues](../.github/ISSUE_TEMPLATE/feature_request.md)
- **📖 Documentation**: All guides are in this `/docs` folder
- **⚙️ Configuration**: Check `.vscode/` for VS Code settings

---

*This documentation is maintained alongside the codebase. When you update code, please update the relevant documentation!* 📝