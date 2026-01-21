cmake_minimum_required(VERSION 3.21)

project(tasks_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Test)
find_package(OpenCV REQUIRED COMPONENTS core)
find_package(OpenMP REQUIRED)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Test executable
add_executable(${PROJECT_NAME}
    test_tasks.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../image_processing/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../image_processing/operations/light
)

# Link to parent build directory libraries
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../build/lib)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Test
    opencv_core
    OpenMP::OpenMP_CXX
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../build/lib/libtasks.a
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../build/lib/libimage_processing.a
)

# Add test to CTest
add_test(NAME TasksTest COMMAND ${PROJECT_NAME})

# Set test properties
set_tests_properties(TasksTest PROPERTIES
    TIMEOUT 30
    LABELS "unit;tasks"
)
