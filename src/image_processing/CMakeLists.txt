# image_processing/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(image_processing LANGUAGES CXX)

# ==========================================
# FIX 1: Build-Type auf Release erzwingen
# ==========================================
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()
# ==========================================

add_library(image_processing STATIC)

# ---------------------------------------------------------
# 1. Plattform & Performance Flags
# ---------------------------------------------------------
if(MSVC)
    target_compile_options(image_processing PRIVATE
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/arch:AVX2>
        /W4
    )
    target_compile_definitions(image_processing PRIVATE NOMINMAX)
else()
    # Linux / Mac (GCC / Clang)
    target_compile_options(image_processing PRIVATE
        $<$<CONFIG:Release>:-O3>
        -Wall -Wextra -Wpedantic
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(image_processing PRIVATE
            # $<$<CONFIG:Release>:-Rpass=loop-vectorize>
            # $<$<CONFIG:Release>:-Rpass-missed=loop-vectorize>
            # $<$<CONFIG:Release>:-Rpass-analysis=loop-vectorize>
            $<$<CONFIG:Release>:-ffast-math>
        )
    endif()

    if(APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        target_compile_options(image_processing PRIVATE 
            $<$<CONFIG:Release>:-mcpu=apple-m1>
        )
    else()
        target_compile_options(image_processing PRIVATE 
            $<$<CONFIG:Release>:-march=native>
        )
    endif()
endif()

# ---------------------------------------------------------
# 2. Dependencies
# ---------------------------------------------------------

# --- FIX: OpenMP für macOS (Homebrew) ---
if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Wir suchen den Pfad von Homebrew libomp
    if(EXISTS "/opt/homebrew/opt/libomp")
        set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
    elseif(EXISTS "/usr/local/opt/libomp")
        set(OpenMP_ROOT "/usr/local/opt/libomp")
    endif()

    if(OpenMP_ROOT)
        message(STATUS "macOS: OpenMP root set to ${OpenMP_ROOT}")
        # Wir geben CMake Hints, damit find_package nicht scheitert
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${OpenMP_ROOT})
        
        # Wir zwingen CMake die Flags auf, die es sonst vermisst
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${OpenMP_ROOT}/include")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY "${OpenMP_ROOT}/lib/libomp.dylib")
        
        include_directories("${OpenMP_ROOT}/include")
        link_directories("${OpenMP_ROOT}/lib")
    endif()
endif()

find_package(OpenMP REQUIRED)

# --- OpenCV ---
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)

# ---------------------------------------------------------
# 3. Quellen
# ---------------------------------------------------------
file(GLOB_RECURSE CORE_SOURCES      CONFIGURE_DEPENDS "core/*.cpp")
file(GLOB_RECURSE OPERATION_SOURCES CONFIGURE_DEPENDS "operations/*.cpp")
file(GLOB_RECURSE UTILS_SOURCES     CONFIGURE_DEPENDS "utils/*.cpp")

target_sources(image_processing PRIVATE
    ${CORE_SOURCES}
    ${OPERATION_SOURCES}
    ${UTILS_SOURCES}
)

# ---------------------------------------------------------
# 4. Includes
# ---------------------------------------------------------
target_include_directories(image_processing PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/operations>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
    $<INSTALL_INTERFACE:include/image_processing>
)

# Wenn wir OpenMP Header gefunden haben, müssen wir das Include Directory auch dem Target geben
if(OpenMP_ROOT)
    target_include_directories(image_processing PRIVATE "${OpenMP_ROOT}/include")
endif()

target_compile_features(image_processing PUBLIC cxx_std_17)

# ---------------------------------------------------------
# 5. Linking
# ---------------------------------------------------------
target_link_libraries(image_processing 
    PUBLIC 
        ${OpenCV_LIBS}
)

# OpenMP Linking Logik
if(OpenMP_CXX_FOUND)
    target_link_libraries(image_processing PRIVATE OpenMP::OpenMP_CXX)
elseif(APPLE AND OpenMP_omp_LIBRARY)
    # Fallback für Mac, falls das Target oben leer ist
    target_link_libraries(image_processing PRIVATE ${OpenMP_omp_LIBRARY})
    target_compile_options(image_processing PRIVATE "-Xpreprocessor" "-fopenmp")
endif()


# ---------------------------------------------------------
# 6. Test & Installation
# ---------------------------------------------------------
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp")
    add_executable(image_processing_test "${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp")
    target_link_libraries(image_processing_test PRIVATE image_processing)
    
    # Auch dem Test executable die Flags geben
    if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
         target_compile_options(image_processing_test PRIVATE "-Xpreprocessor" "-fopenmp")
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_data")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
endif()

install(TARGETS image_processing
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY core/ operations/ utils/
    DESTINATION include/image_processing
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# ---------------------------------------------------------
# STATUS CHECK (Damit du sicher bist!)
# ---------------------------------------------------------
message(STATUS "---------------------------------------------------")
message(STATUS ">> BUILD CONFIGURATION CHECK <<")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "PERFORMANCE: ✅  OPTIMIZATIONS ON (-O3 active)")
else()
    message(STATUS "PERFORMANCE: ❌  SLOW MODE DETECTED (Debug active)")
endif()
message(STATUS "---------------------------------------------------")