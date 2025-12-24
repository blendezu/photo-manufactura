# ============================================================
# Photo Manufactura - UI Component
# ============================================================
# Can be built standalone (app) or as a submodule (library)
# ============================================================

cmake_minimum_required(VERSION 3.21)

# ============================================================
# Detect Build Mode
# ============================================================
set(STANDALONE_BUILD OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(STANDALONE_BUILD ON)
endif()

# ============================================================
# Project Configuration (Standalone Only)
# ============================================================
if(STANDALONE_BUILD)
    project(ui_component 
        VERSION 1.0.0 
        LANGUAGES CXX
        DESCRIPTION "Photo Manufactura UI Component"
    )

    # C++ Standard
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # Qt Meta-Object Compiler
    set(CMAKE_AUTOMOC ON)  # Automatically run moc on Qt classes
    set(CMAKE_AUTORCC ON)  # Automatically compile .qrc files
    set(CMAKE_AUTOUIC ON)  # Automatically compile .ui files
endif()

# ============================================================
# Dependencies
# ============================================================
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    OpenGL 
    OpenGLWidgets
)

# ============================================================
# UI Library Target
# ============================================================
add_library(ui STATIC)

# ------------------------------------------------------------
# Source Files Organization
# ------------------------------------------------------------
set(MAIN_SOURCES
    mainwindow.h
    mainwindow.cpp
)

set(BAR_SOURCES
    bar/toolBar.h
    bar/toolBar.cpp
    bar/subMenuFile.h
    bar/subMenuFile.cpp
    bar/subMenuEdit.h
    bar/subMenuEdit.cpp
    bar/subMenuView.h
    bar/subMenuView.cpp
)

set(PANEL_SOURCES
    panel/infoPanel.h
    panel/infoPanel.cpp
    panel/toolPanel.h
    panel/toolPanel.cpp
)

set(WIDGET_SOURCES
    widgets/collapsibleWidget.h
    widgets/collapsibleWidget.cpp
    widgets/labeledSlider.h
    widgets/labeledSlider.cpp
    widgets/toolPaletteWidget.h
    widgets/toolPaletteWidget.cpp
)

set(CANVAS_SOURCES
    canvas/canvasWidget.h
    canvas/canvasWidget.cpp
)

set(THEME_SOURCES
    resources/theme/styleSheet.h
    resources/theme/styleSheet.cpp
    resources/theme/themeManager.h
    resources/theme/themeManager.cpp
)

set(RESOURCE_SOURCES
    resources/resources.qrc
)

# Add all sources to target
target_sources(ui PRIVATE
    ${MAIN_SOURCES}
    ${BAR_SOURCES}
    ${PANEL_SOURCES}
    ${WIDGET_SOURCES}
    ${CANVAS_SOURCES}
    ${THEME_SOURCES}
    ${RESOURCE_SOURCES}
)

# ------------------------------------------------------------
# Include Directories
# ------------------------------------------------------------
target_include_directories(ui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bar>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/canvas>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/panel>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/resources/theme>
    $<INSTALL_INTERFACE:include>
)

# ------------------------------------------------------------
# Link Libraries
# ------------------------------------------------------------
target_link_libraries(ui PUBLIC 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets 
    Qt6::OpenGL 
    Qt6::OpenGLWidgets
)

# ============================================================
# Standalone Executable (Optional)
# ============================================================
if(STANDALONE_BUILD)
    # Add model and controller as subdirectories for standalone build
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../model ${CMAKE_BINARY_DIR}/model)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../controller ${CMAKE_BINARY_DIR}/controller)
    
    # Create executable
    add_executable(ui_main
        ui_main.cpp
    )

    # Link with UI library, controller and model
    target_link_libraries(ui_main PRIVATE 
        ui
        controller
        model
    )
    
    # Add controller and model include directories
    target_include_directories(ui_main PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../controller
        ${CMAKE_CURRENT_SOURCE_DIR}/../model
    )

    # Output directory
    set_target_properties(ui_main PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    message(STATUS "✅ Building standalone UI executable (entry: ui_main.cpp)")
else()
    message(STATUS "📦 Building UI as a submodule library")
endif()
