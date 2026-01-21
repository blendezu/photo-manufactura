# 📚 Documentation Index

Welcome to Photo Manufactura documentation!

## 🚀 Quick Start

| Guide | Description |
|-------|-------------|
| [Project README](../README.md) | Overview & quick start |
| [Build Instructions](BUILD.md) | Setup & build guide |
| [VS Code Setup](vscode/setup.md) | IDE configuration |

## 🛠️ Development

| Guide | Description |
|-------|-------------|
| [CMake Guide](development/cmake.md) | Build system & presets |
| [Component Guide](development/components.md) | Architecture & patterns |
| [Troubleshooting](troubleshooting.md) | Common issues & fixes |

## 🏗️ Architecture (MVC)

| Layer | Documentation |
|-------|---------------|
| **Model** | [Model Layer](development/Model.md) - Document state, settings, app state |
| **View** | [UI Component](../src/ui/README.md) - Qt6 widgets and panels |
| **Controller** | [Components Guide](development/components.md) - Business logic orchestration |

## 🖼️ Image Processing

| Topic | Documentation |
|-------|---------------|
| Pipeline | [ImagePipeline](development/ImagePipeline_v3.md) - Non-destructive editing |
| Operations | [OperationRegistry](development/OperationRegistry_v2.md) - Filter management |

## ⌨️ Quick Commands

\`\`\`bash
# Build project
cmake --preset dev && cmake --build --preset dev

# Build component
cd src/ui && cmake --preset debug && cmake --build --preset debug

# Clean rebuild
rm -rf build/ src/*/build/ && cmake --preset debug
\`\`\`

## 🆘 Help

- **Build issues**: [troubleshooting.md](troubleshooting.md)
- **Bug reports**: [GitHub Issues](../.github/ISSUE_TEMPLATE/bug_report.md)
- **Features**: [GitHub Issues](../.github/ISSUE_TEMPLATE/feature_request.md)
