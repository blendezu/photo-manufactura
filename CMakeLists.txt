cmake_minimum_required(VERSION 3.16)
project(photo_manufactura
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefer Ninja
if(NOT CMAKE_GENERATOR)
    set(CMAKE_GENERATOR "Ninja" CACHE STRING "")
endif()

# Enable AUTOMOC, AUTORCC, AUTOUIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Debug info
message(STATUS "Using Qt version ${QT_VERSION}")
message(STATUS "Project: ${PROJECT_NAME} Version: ${PROJECT_VERSION}")
message(STATUS "OpenCV Version: ${OpenCV_VERSION}")

add_executable(${PROJECT_NAME}
    src/main.cpp
)

add_subdirectory(
    src/ui
)
add_subdirectory(
    src/scheduler_worker
)
add_subdirectory(
    src/raw_processing
)
add_subdirectory(src/image_processing)
add_subdirectory(src/controller)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

