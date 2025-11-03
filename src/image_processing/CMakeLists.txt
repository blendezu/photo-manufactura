# image_processing/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(image_processing LANGUAGES CXX)

# Library erstellen
add_library(image_processing STATIC)

# Alle Quell-Dateien sammeln
file(GLOB_RECURSE CORE_SOURCES "core/*.cpp")
file(GLOB_RECURSE OPERATION_SOURCES "operations/*.cpp") 
file(GLOB_RECURSE UTILS_SOURCES "utils/*.cpp")

# Quellen zur Library hinzufügen
target_sources(image_processing PRIVATE
    ${CORE_SOURCES}
    ${OPERATION_SOURCES}
    ${UTILS_SOURCES}
)

# Include-Verzeichnisse
target_include_directories(image_processing PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/operations>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
)

# C++20 Standard
target_compile_features(image_processing PUBLIC cxx_std_20)

# OpenCV finden und verlinken
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)

if(TARGET OpenCV::OpenCV)
    target_link_libraries(image_processing PUBLIC OpenCV::OpenCV)
elseif(TARGET OpenCV::opencv_core)
    target_link_libraries(image_processing PUBLIC
        OpenCV::opencv_core
        OpenCV::opencv_imgproc
        OpenCV::opencv_imgcodecs
        OpenCV::opencv_highgui
    )
else()
    target_include_directories(image_processing PUBLIC ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(image_processing PUBLIC ${OpenCV_LIBS})
endif()

# Compiler-Warnungen
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(image_processing PRIVATE 
        $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -Werror>
        $<$<CONFIG:Release>:-O3>
    )
endif()

# Test-Executable
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp")
    add_executable(image_processing_test "${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp")
    target_link_libraries(image_processing_test PRIVATE image_processing)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_data")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_data"
             DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
endif()

# Installations-Ziel (optional)
install(TARGETS image_processing
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY core/ operations/ utils/
    DESTINATION include/image_processing
    FILES_MATCHING PATTERN "*.h"
)