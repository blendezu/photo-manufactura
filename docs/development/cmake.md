# CMake Development Guide

Advanced CMake configuration and optimization for Photo Manufactura.

## CMakeLists.txt Template

Standard pattern for component CMakeLists.txt:

```cmake
# Standalone mode detection
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.21)
    project(component_name VERSION 1.0.0 LANGUAGES CXX)
    
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    
    # Qt components need MOC
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# Create library
add_library(component_name STATIC)
target_sources(component_name PRIVATE source.cpp source.h)
target_include_directories(component_name PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Dependencies
find_package(Qt6 COMPONENTS Core Widgets)
if(Qt6_FOUND)
    target_link_libraries(component_name PUBLIC Qt6::Core Qt6::Widgets)
endif()

# Test executable (standalone only)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(test_component test_main.cpp)
    target_link_libraries(test_component PRIVATE component_name)
endif()
```

## Build Optimization

### Compiler Flags

```cmake
# Release
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

# Debug
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer")

# Link-time optimization
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
```

### Build Speed

| Technique | Configuration |
|-----------|---------------|
| Ninja | `cmake.generator": "Ninja"` |
| Parallel | `cmake --build . -j$(nproc)` |
| ccache | `-DCMAKE_CXX_COMPILER_LAUNCHER=ccache` |
| Unity builds | `set(CMAKE_UNITY_BUILD ON)` |

### Sanitizers

```bash
# Address sanitizer
cmake --preset dev -DCMAKE_CXX_FLAGS="-fsanitize=address"

# Thread sanitizer
cmake --preset dev -DCMAKE_CXX_FLAGS="-fsanitize=thread"

# Static analysis
cmake --preset dev -DCMAKE_CXX_CLANG_TIDY=clang-tidy
```

## Testing Configuration

```cmake
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    enable_testing()
    add_test(NAME component_tests COMMAND test_component)
    set_tests_properties(component_tests PROPERTIES TIMEOUT 30)
endif()
```

```bash
# Run tests
ctest --preset default
ctest --preset default --verbose
ctest --preset parallel-tests
```

## Custom Variables

```bash
cmake --preset default -DENABLE_BENCHMARKS=ON
cmake --preset default -DENABLE_DOCS=ON
cmake --preset default -DBUILD_SHARED_LIBS=ON
cmake --preset default -DCMAKE_INSTALL_PREFIX=/usr/local
```

## Packaging

```bash
cmake --preset default
cmake --build --preset default
cmake --install build/default --prefix /usr/local
cpack --config build/default/CPackConfig.cmake
```
