cmake_minimum_required(VERSION 3.16)

project(model LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui)

# Find OpenCV (required for image processing integration)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

# Model sources
set(MODEL_SOURCES
    ImageDocument.cpp
    AdjustmentSettings.cpp
    AppState.cpp
    DocumentManager.cpp
    PresetManager.cpp
)

set(MODEL_HEADERS
    ImageDocument.h
    AdjustmentSettings.h
    AppState.h
    DocumentManager.h
    PresetManager.h
)

# Create library
add_library(model STATIC ${MODEL_SOURCES} ${MODEL_HEADERS})

# Use QT_NO_KEYWORDS to avoid conflict with Halide's emit() method
target_compile_definitions(model PRIVATE QT_NO_KEYWORDS)

target_include_directories(model PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# image_processing headers are accessed via target's PUBLIC include directories
target_link_libraries(model PUBLIC
    Qt6::Core
    Qt6::Gui
    image_processing
    ${OpenCV_LIBS}
)
