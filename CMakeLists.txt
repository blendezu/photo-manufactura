cmake_minimum_required(VERSION 3.21)
project(photo_manufactura
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Prefer Ninja
if(NOT CMAKE_GENERATOR)
    set(CMAKE_GENERATOR "Ninja" CACHE STRING "")
endif()

# Enable AUTOMOC, AUTORCC, AUTOUIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)



add_executable(${PROJECT_NAME}
    src/main.cpp
)

# TODO: Add subdirectories when you have source files in them
add_subdirectory(src/ui)
# add_subdirectory(src/scheduler_worker)
# add_subdirectory(src/raw_processing)
# add_subdirectory(src/image_processing)
# add_subdirectory(src/controller)

# TODO: Link libraries when they exist
target_link_libraries(${PROJECT_NAME} PRIVATE
    ui
#     scheduler_worker
#     raw_processing
#     image_processing
#     controller
)

# Add compile options for better debugging and warnings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall -Wextra -Wpedantic -g
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

