cmake_minimum_required(VERSION 3.21)
project(photo_manufactura
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Enable AUTOMOC, AUTORCC, AUTOUIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Bessere Ausgabe-Verzeichnis-Struktur
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# macOS App Bundle configuration
set(APP_ICON_MACOSX ${CMAKE_SOURCE_DIR}/resources/icon.icns)

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set(MACOSX_BUNDLE_BUNDLE_NAME "Photo Manufactura")
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2025")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.photomanufactura.app")
    
    # Set icon file properties if it exists
    if(EXISTS ${APP_ICON_MACOSX})
        set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
    endif()
endif()

if(WIN32)
    # Windows-specific settings can be added here
endif()

if(UNIX AND NOT APPLE)
    # Linux-specific settings can be added here
endif()

# Prepare source files list
set(APP_SOURCES src/main.cpp)
if(APPLE AND EXISTS ${APP_ICON_MACOSX})
    list(APPEND APP_SOURCES ${APP_ICON_MACOSX})
endif()

# Create executable
if(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${APP_SOURCES})
else()
    add_executable(${PROJECT_NAME} ${APP_SOURCES})
endif()

# Subdirectories - order matters for dependencies
# image_processing must be built before model (model depends on it)
add_subdirectory(src/image_processing)
add_subdirectory(src/raw_processing)
add_subdirectory(src/tasks)
add_subdirectory(src/model)
add_subdirectory(src/controller)
add_subdirectory(src/ui)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    model
    raw_processing
    image_processing
    tasks
    controller
    ui
)

# Warnungen für alle Targets einheitlich setzen
target_compile_options(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -g>
)

install(TARGETS ${PROJECT_NAME} 
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Install AI models
if(EXISTS "${CMAKE_SOURCE_DIR}/AI_models")
    if(APPLE)
        install(DIRECTORY "${CMAKE_SOURCE_DIR}/AI_models" 
                DESTINATION "${PROJECT_NAME}.app/Contents/Resources")
    else()
        install(DIRECTORY "${CMAKE_SOURCE_DIR}/AI_models" 
                DESTINATION bin)
    endif()
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/AI_models")
    message(STATUS "Copying Style Transfer Models to bin folder...")
    # Copy AI models to output directory
    file(COPY "${CMAKE_SOURCE_DIR}/AI_models" 
         DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
    message(WARNING "⚠️ Folder 'AI_models' not found in root!")
endif()