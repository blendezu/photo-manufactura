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

## 🧩 Components

| Component | Documentation |
|-----------|---------------|
| UI | [src/ui/README.md](../src/ui/README.md) |
| Image Processing | [src/image_processing/](../src/image_processing/) |
| Controller | [src/controller/](../src/controller/) |

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
