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

add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Subdirectories
add_subdirectory(src/raw_processing)
add_subdirectory(src/image_processing)
# Weitere Module später hinzufügen

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    raw_processing
    image_processing
)

# Warnungen für alle Targets einheitlich setzen
target_compile_options(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -g>
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

if(EXISTS "${CMAKE_SOURCE_DIR}/AI_models")
    message(STATUS "Copying Style Transfer Models to bin folder...")
    file(COPY "${CMAKE_SOURCE_DIR}/AI_models" 
         DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
    message(WARNING "⚠️ Folder 'AI_models' not found in root!")
endif()