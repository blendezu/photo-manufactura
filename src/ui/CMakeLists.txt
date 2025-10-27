# ============================================================
# UI Component
# Can be built standalone (app) or as a submodule (library)
# ============================================================

cmake_minimum_required(VERSION 3.21)

# Detect if built standalone
set(STANDALONE_BUILD OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(STANDALONE_BUILD ON)
endif()

if(STANDALONE_BUILD)
    project(ui_component VERSION 1.0.0 LANGUAGES CXX)

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # Qt tools for MOC, UIC, RCC
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# ============================================================
# Find Qt dependencies
# ============================================================
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

# ============================================================
# UI Library
# ============================================================
add_library(ui STATIC)

target_sources(ui PRIVATE
    mainwindow.cpp
    mainwindow.h
    bar/toolBar.cpp
    bar/toolBar.h
    bar/subMenuFile.cpp
    bar/subMenuFile.h
    panel/infoPanel.cpp
    panel/infoPanel.h
    panel/toolPanel.cpp
    panel/toolPanel.h
    widgets/canvasWidget.cpp
    widgets/canvasWidget.h
)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ui PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)

# ============================================================
# Standalone Executable
# ============================================================
if(STANDALONE_BUILD)
    add_executable(ui_main
        ui_main.cpp
    )

    target_link_libraries(ui_main PRIVATE ui)

    set_target_properties(ui_main PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    message(STATUS "✅ Building standalone UI executable (entry: ui_main.cpp)")
else()
    message(STATUS "📦 Building UI as a submodule library")
endif()
