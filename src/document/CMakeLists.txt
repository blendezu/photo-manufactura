cmake_minimum_required(VERSION 3.21)

project(document LANGUAGES CXX)

set(DOCUMENT_SOURCES
    document.cpp
    document.h
)

add_library(document STATIC ${DOCUMENT_SOURCES})
target_compile_features(document PUBLIC cxx_std_20)

find_package(OpenCV REQUIRED)
find_package(Qt6 COMPONENTS Gui REQUIRED)

target_include_directories(document
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(document
    PUBLIC
        Qt6::Gui
        ${OpenCV_LIBS}
)

target_compile_options(document PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -g>
)

install(TARGETS document
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)