# VS Code Setup

VS Code configuration for Photo Manufactura development.

## Extensions

Install from `.vscode/extensions.json`:

| Extension | ID | Purpose |
|-----------|-----|---------|
| CMake Tools | `ms-vscode.cmake-tools` | CMake integration |
| C/C++ | `ms-vscode.cpptools` | IntelliSense |
| CMake | `twxs.cmake` | CMake syntax |
| clangd | `llvm-vs-code-extensions.vscode-clangd` | C++ language server |

## Configuration Files

```
.vscode/
├── settings.json      # VS Code settings
├── tasks.json         # Build tasks
├── launch.json        # Debug configs
├── cmake-kits.json    # Compilers
└── extensions.json    # Extensions
```

## Key Settings

```json
{
    "cmake.useCMakePresets": "always",
    "cmake.configureOnOpen": true,
    "cmake.generator": "Ninja",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

## Multi-Root Workspace

Open `photo-manufactura.code-workspace`:

```
📁 Main Project          # Full project
📁 UI Component          # src/ui/
📁 Controller Component  # src/controller/
📁 Image Processing      # src/image_processing/
```

## Debugging

### Configurations

- **Debug Main App** - Full application
- **Debug UI Component** - Standalone UI
- **Debug Controller** - Standalone controller

### Workflow

1. Set breakpoints
2. Select configuration (F5 dropdown)
3. Press F5
4. Step through code

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Cmd+Shift+B` | Build |
| `Cmd+Shift+P` → `CMake: Configure` | Configure |
| `F5` | Debug |
| `Cmd+Shift+P` → `Tasks: Run Task` | Run task |

## Quick Start

1. Open project root in VS Code
2. Install recommended extensions
3. Select CMake kit (Clang)
4. Choose preset (`dev`)
5. Build (`Cmd+Shift+B`)
6. Debug (`F5`)
