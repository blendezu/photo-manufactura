# 📸 Photo Manufactura

> **High-performance, modular photo processing engine built with modern C++20 and Qt6**

[![CMake](https://img.shields.io/badge/CMake-3.21+-blue.svg)](https://cmake.org/)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.x-green.svg)](https://www.qt.io/)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.x-orange.svg)](https://opencv.org/)
[![LibRaw](https://img.shields.io/badge/LibRaw-0.20.2-blue.svg)](https://github.com/LibRaw/LibRaw)
[![License](https://img.shields.io/badge/license-GNU-blue.svg)](LICENSE)

## 🚀 Quick Start

### Prerequisites
- **CMake 3.21+**
- **C++20 compatible compiler** (GCC 10+, Clang 12+, MSVC 2019+)
- **Qt6** (GUI components)
- **OpenCV** (advanced image processing)
- **LibRaw** (RAW file support)

### Installation

```bash
# Clone the repository
git clone https://github.com/adt97vn/photo-manufactura.git
cd photo-manufactura

# Build and run
cmake --preset default
cmake --build --preset default
./build/default/bin/photo_manufactura
```

> 📖 **Detailed build instructions**: See [docs/BUILD.md](docs/BUILD.md)

## 📁 Project Structure

```
photo-manufactura/
├── 📄 BUILD.md                    # Detailed build instructions
├── ⚙️  CMakeLists.txt              # Main CMake configuration
├── 🔧 CMakePresets.json           # Build presets (default, debug, dev)
├── 📋 conan.txt                   # Package dependencies
├── 📜 LICENSE                     # Project license
├── 📖 README.md                   # This file
├── 🔨 build/                      # Build outputs (generated)
│   ├── default/                   # Release build artifacts
│   │   └── bin/photo_manufactura  # Main executable
│   └── dev/                       # Development build artifacts
└── 📦 src/                        # Source code
    ├── 🖥️  controller/             # Application logic & coordination
    │   └── CMakeLists.txt
    ├── 📄 document/               # Document management system
    │   └── CMakeLists.txt
    ├── 🎨 image_processing/       # Core image manipulation algorithms
    │   └── CMakeLists.txt
    ├── 📸 raw_processing/         # RAW file format handling
    │   └── CmakeLists.txt
    ├── ⚡ scheduler_worker/       # Task scheduling & worker threads
    │   └── CMakeLists.txt
    ├── 🖼️  ui/                     # Qt6-based user interface
    │   ├── CMakeLists.txt         # UI component build config
    │   ├── ui_main.cpp            # UI implementation
    │   ├── ui_main.h              # UI interface definitions
    │   └── test_main.cpp          # UI component tests
    └── 🚀 main.cpp                # Application entry point
```

### 🏗️ Architecture Overview

**Modular Design**: Each component in `src/` is independently buildable and testable:

| Component | Purpose | Key Features |
|-----------|---------|-------------|
| **UI** | User interface layer | Qt6 widgets, responsive design |
| **Controller** | Application coordination | MVC pattern, event handling |
| **Document** | File & project management | Session state, project files |
| **Image Processing** | Core algorithms | Filters, transforms, effects |
| **RAW Processing** | Professional workflow | LibRaw integration, metadata |
| **Scheduler Worker** | Performance optimization | Multi-threading, task queues |

### Component Independence
Each component can be developed, tested, and built independently:

```bash
cd src/ui
cmake -B build -S . && cmake --build build
./build/bin/ui_test
```

## 🎯 Use Cases

### Professional Photography Editing

- **RAW Workflow**: Import, process, and export professional RAW files
- **Color Management**: Professional color space handling and calibration
- **Filter Library**: Extensive collection of image filters and effects
- **Basic Editing Tools**: Crop, rotate, adjust brightness/contrast
  

## 🛠️ Development

### Build Configurations

| Preset | Purpose | Optimization | Output |
|--------|---------|-------------|---------|
| `default` | Standard release build | `-O3` | `build/default/` |
| `debug` | Development with symbols | `-g -O0` | `build/debug/` |
| `dev` | Debug + extra warnings | `-Wall -Wextra` | `build/dev/` |

### Code Quality
- **Modern C++20** features and best practices
- **Automatic formatting** with clang-format
- **Static analysis** integration
- **Comprehensive testing** with independent component tests

### Contributing
1. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
2. **Commit** your changes (`git commit -m '<prefix> : Add amazing feature'`) with `feat`, `fix`, or `docs` prefixes
3. **Push** to the branch (`git push origin feature/amazing-feature`)
4. **Open** a Pull Request
5. **Review** and discuss changes with maintainers
6. **Merge** after approval and passing tests
7. **Celebrate** your contribution to Photo Manufactura! 

## 📊 Performance

To be Added Soon! 

## 🤝 Community & Support

- **📖 Documentation**: [Build Guide](docs/BUILD.md) | [API Reference](docs/)
- **🐛 Issues**: [GitHub Issues](https://github.com/adt97vn/photo-manufactura/issues)
- **💬 Discussions**: [GitHub Discussions](https://github.com/adt97vn/photo-manufactura/discussions)


## 📄 License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.


---

<div align="center">

**[⭐ Star this repo](https://github.com/adt97vn/photo-manufactura)** • **[🍴 Fork it](https://github.com/adt97vn/photo-manufactura/fork)** • **[📖 Read the docs](docs/BUILD.md)**

*Built with ❤️ for photographers and developers*

</div>