cmake_minimum_required(VERSION 3.16)

project(model LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui)

# Model sources
set(MODEL_SOURCES
    ImageDocument.cpp
    AdjustmentSettings.cpp
    AppState.cpp
    DocumentManager.cpp
)

set(MODEL_HEADERS
    ImageDocument.h
    AdjustmentSettings.h
    AppState.h
    DocumentManager.h
)

# Create library
add_library(model STATIC ${MODEL_SOURCES} ${MODEL_HEADERS})

target_include_directories(model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(model PUBLIC
    Qt6::Core
    Qt6::Gui
)
