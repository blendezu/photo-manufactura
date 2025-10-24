# 📸 Photo Manufactura

> **A modern, modular C++ application for professional photo processing and manufacturing workflows**

[![CMake](https://img.shields.io/badge/CMake-3.21+-blue.svg)](https://cmake.org/)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.x-green.svg)](https://www.qt.io/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

## ✨ Features

- 🎯 **Modular Architecture** - Independent, testable components
- 🚀 **High Performance** - Modern C++20 with optimized processing
- 🖼️ **RAW Processing** - Professional RAW file handling and conversion
- 🎨 **Advanced Image Processing** - Comprehensive editing and enhancement tools
- 🔄 **Workflow Automation** - Intelligent task scheduling and batch processing
- 💻 **Cross-Platform** - Runs on macOS, Linux, and Windows
- 🧪 **Extensively Tested** - Component-level and integration testing

## 🚀 Quick Start

### Prerequisites
- **CMake 3.21+**
- **C++20 compatible compiler** (GCC 10+, Clang 12+, MSVC 2019+)
- **Qt6** (optional, for GUI components)
- **OpenCV** (optional, for advanced image processing)

### Installation

```bash
# Clone the repository
git clone https://github.com/ad-tran/photo-manufactura.git
cd photo-manufactura

# Build and run
cmake --preset default
cmake --build --preset default
./build/default/bin/photo_manufactura
```

> 📖 **Detailed build instructions**: See [BUILD.md](BUILD.md)

## 🏗️ Architecture

Photo Manufactura is built with a modular, component-based architecture:

```
📁 src/
├── 🎨 ui/                 # User interface components
├── 🎮 controller/         # Application logic and coordination
├── 🖼️  image_processing/   # Core image manipulation algorithms
├── 📷 raw_processing/     # RAW file format handling
└── ⚡ scheduler_worker/   # Task scheduling and worker threads
```

### Component Independence
Each component can be developed, tested, and built independently:

```bash
cd src/ui
cmake -B build -S . && cmake --build build
./build/bin/ui_test
```

## 🎯 Use Cases

### Professional Photography
- **RAW Workflow**: Import, process, and export professional RAW files
- **Batch Processing**: Apply edits to thousands of images automatically
- **Color Management**: Professional color space handling and calibration

### Manufacturing & Production
- **Quality Control**: Automated image analysis for defect detection
- **Workflow Integration**: Seamless integration with production pipelines
- **Performance Optimization**: High-throughput processing for industrial use

### Development & Research
- **Plugin Architecture**: Extensible framework for custom algorithms
- **API Integration**: Easy integration with external tools and services
- **Research Platform**: Foundation for computer vision and image processing research

## 🛠️ Development

### Build Configurations

| Preset | Purpose | Optimization |
|--------|---------|-------------|
| `default` | Standard release build | `-O3` |
| `debug` | Development with symbols | `-g -O0` |
| `dev` | Debug + extra warnings | `-Wall -Wextra` |
| `release-optimized` | Maximum performance | `-O3 -march=native` |

### Code Quality
- **Modern C++20** features and best practices
- **Automatic formatting** with clang-format
- **Static analysis** integration
- **Comprehensive testing** with independent component tests

### Contributing
1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

## 📊 Performance

Photo Manufactura is designed for high-performance photo processing:

- **Multi-threaded**: Efficient use of modern multi-core processors
- **Memory Optimized**: Smart memory management for large image files
- **SIMD Optimized**: Vectorized operations for critical image processing paths
- **GPU Ready**: Architecture supports future GPU acceleration

## 🤝 Community & Support

- **📖 Documentation**: [Build Guide](BUILD.md) | [API Reference](docs/)
- **🐛 Issues**: [GitHub Issues](https://github.com/ad-tran/photo-manufactura/issues)
- **💬 Discussions**: [GitHub Discussions](https://github.com/ad-tran/photo-manufactura/discussions)
- **📧 Contact**: [maintainer@photo-manufactura.dev](mailto:maintainer@photo-manufactura.dev)

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Qt Framework** for excellent cross-platform UI capabilities
- **OpenCV** for comprehensive computer vision algorithms
- **CMake** for modern, flexible build system
- **C++ Community** for continuous language evolution

---

<div align="center">

**[⭐ Star this repo](https://github.com/ad-tran/photo-manufactura)** • **[🍴 Fork it](https://github.com/ad-tran/photo-manufactura/fork)** • **[📖 Read the docs](BUILD.md)**

*Built with ❤️ for photographers and developers*

</div>