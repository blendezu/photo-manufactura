# UI Component - Can be built independently or as part of main project
# This file works in both standalone and subdirectory mode

# Check if this is being built as a standalone project
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # Standalone build
    cmake_minimum_required(VERSION 3.21)
    project(ui_component VERSION 1.0.0 LANGUAGES CXX)
    
    # Set C++ standard for standalone build
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # Enable compile commands for IDEs
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Create the UI library
add_library(ui STATIC)

# Add source files
target_sources(ui PRIVATE
    ui_main.cpp
    ui_main.h
)

# Include directories
target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Dependencies (Qt6 with error handling)
find_package(Qt6 COMPONENTS Core Widgets)
if(Qt6_FOUND)
    target_link_libraries(ui PUBLIC Qt6::Core Qt6::Widgets)
    message(STATUS "Qt6 found: ${Qt6_VERSION}")
else()
    message(WARNING "Qt6 not found - building without Qt support")
endif()

# If standalone build, create an executable for testing
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # Create a test executable for this component
    add_executable(ui_test)
    
    target_sources(ui_test PRIVATE
        test_main.cpp
    )
    
    target_link_libraries(ui_test PRIVATE ui)
    
    # Set output directory
    set_target_properties(ui_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()
