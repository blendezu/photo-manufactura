# Controller Component - Can be built independently or as part of main project

# Check if this is being built as a standalone project
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # Standalone build
    cmake_minimum_required(VERSION 3.21)
    project(controller_component VERSION 1.0.0 LANGUAGES CXX)
    
    # Set C++ standard for standalone build
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # Enable compile commands for IDEs
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Create the Controller library
add_library(controller STATIC)

# Add source files when you create them
# target_sources(controller PRIVATE
#     controller.cpp
#     controller_manager.cpp
#     event_handler.cpp
# )

# Include directories
target_include_directories(controller PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Dependencies
# target_link_libraries(controller PUBLIC
#     # Add any dependencies here
# )

# If standalone build, create an executable for testing
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # Create a test executable for this component
    add_executable(controller_test)
    
    # target_sources(controller_test PRIVATE
    #     test_main.cpp  # Create this for testing
    # )
    
    target_link_libraries(controller_test PRIVATE controller)
    
    # Set output directory
    set_target_properties(controller_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()
