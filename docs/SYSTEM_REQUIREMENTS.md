# Photo Manufactura - System Requirements

## Minimum Requirements

### macOS
- **OS Version**: macOS 11.0 (Big Sur) or later
- **Processor**: Apple Silicon (M1/M2/M3) or Intel Core i5 (2015+)
- **RAM**: 4 GB
- **Storage**: 500 MB free space (+ space for your photos)
- **Graphics**: Metal-compatible GPU

### Linux
- **Distribution**: Ubuntu 20.04+ / Fedora 35+ / Debian 11+ or equivalent
- **Kernel**: 5.4+
- **Processor**: x86_64, 2 cores minimum
- **RAM**: 4 GB
- **Storage**: 500 MB free space
- **Graphics**: OpenGL 3.3+ support
- **Display Server**: X11 or Wayland

### Windows
- **OS Version**: Windows 10 (1909+) or Windows 11
- **Processor**: Intel Core i5 / AMD Ryzen 5 or better
- **RAM**: 4 GB
- **Storage**: 500 MB free space
- **Graphics**: DirectX 11 compatible GPU

## Recommended Requirements

### All Platforms
- **Processor**: Multi-core (4+ cores) for faster processing
- **RAM**: 8 GB or more (16 GB for large images)
- **Storage**: SSD with 2+ GB free space
- **Graphics**: Dedicated GPU with 2+ GB VRAM
- **Display**: 1920x1080 or higher resolution

## Software Dependencies

### macOS
Automatically bundled in the app:
- Qt 6.8.3+
- OpenCV 4.12.0+
- LibRaw 0.21.2+
- ONNX Runtime 1.16.3+

### Linux
Required libraries (auto-installed with package managers):
- Qt6 Base, Widgets, OpenGL, OpenGLWidgets
- OpenCV 4.x (core, imgproc, imgcodecs, highgui, dnn)
- LibRaw 0.21+
- ONNX Runtime 1.16+
- libomp (OpenMP runtime)

Install on Ubuntu/Debian:
```bash
sudo apt install qt6-base-dev libopencv-dev libraw-dev
```

Install on Fedora:
```bash
sudo dnf install qt6-qtbase-devel opencv-devel LibRaw-devel
```

### Windows
All dependencies bundled in the installer.

## AI Model Requirements

Photo Manufactura includes AI models for:
- Image denoising
- Style transfer
- Background removal

**Storage**: ~300 MB for all AI models (included in distribution)

## Supported Image Formats

### Input Formats
- **RAW**: CR2, NEF, ARW, DNG, RAF, ORF, RW2, PEF, and more
- **Standard**: JPG, JPEG, PNG, TIFF, BMP, WebP
- **High Bit Depth**: 16-bit TIFF, PNG

### Output Formats
- JPG (8-bit)
- PNG (8-bit, 16-bit)
- TIFF (8-bit, 16-bit)
- WebP

## Performance Notes

### Image Size Limits
- **Maximum resolution**: Limited by available RAM
  - 4 GB RAM: ~25 MP images
  - 8 GB RAM: ~50 MP images
  - 16 GB RAM: ~100+ MP images

### Processing Speed
Depends on:
- CPU/GPU performance
- Image resolution
- Applied operations
- AI model complexity

Typical processing times (24 MP image on M1 MacBook):
- Basic adjustments: <1 second
- AI denoising: 2-5 seconds
- Style transfer: 3-8 seconds
- RAW conversion: 1-3 seconds

## Network Requirements

- **Installation**: Internet connection for downloading (20-100 MB)
- **Operation**: No internet required (works offline)
- **Updates**: Internet for checking/downloading updates

## Disk Space by Component

- Application binary: ~50 MB
- Qt frameworks: ~100 MB (macOS/Windows bundled)
- OpenCV libraries: ~50 MB
- AI models: ~300 MB
- **Total**: ~500 MB

Plus space for:
- User photos
- Temporary processing files
- Export cache

## Troubleshooting

### macOS "Cannot open app from unidentified developer"
```bash
xattr -cr /Applications/photo_manufactura.app
```
Or right-click → Open

### Linux missing libraries
```bash
ldd photo_manufactura  # Check missing dependencies
```

### Slow performance
- Close other applications
- Process smaller batches
- Reduce output resolution
- Disable GPU acceleration if unstable

### Out of memory errors
- Close other applications
- Reduce image size before processing
- Process images individually vs batch
- Increase system swap space

## Compatibility Testing

Tested on:
- ✅ macOS 14 Sonoma (Apple Silicon)
- ✅ macOS 13 Ventura (Intel)
- ✅ Ubuntu 22.04 LTS
- ✅ Fedora 39
- ⚠️  Windows 11 (limited testing)

## Future Requirements

Planned features may require:
- GPU compute (CUDA/Metal/OpenCL) for faster AI
- Additional AI models (more disk space)
- Cloud sync features (network access)
