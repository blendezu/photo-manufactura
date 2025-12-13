cmake_minimum_required(VERSION 3.20)
project(image_processing LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==========================================
# FIX: RPATH für lokale Libs (Wichtig für macOS!)
# ==========================================
# Damit das Programm beim Start weiß, wo die .dylib Dateien liegen
set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
# Füge hier beide Pfade hinzu (ONNX und Halide)
set(CMAKE_INSTALL_RPATH "${CMAKE_SOURCE_DIR}/libs/Halide/lib;${CMAKE_SOURCE_DIR}/libs/onnxruntime/lib")

# ==========================================
# FIX 1: Build-Type wird nun über CMakePresets gesteuert
# ==========================================
# if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
#     message(STATUS "Setting build type to 'Release' as none was specified.")
#     set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
# endif()

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
    target_compile_options(image_processing PRIVATE
        $<$<CONFIG:Release>:-O3>
        -Wall -Wextra -Wpedantic
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(image_processing PRIVATE
            $<$<CONFIG:Release>:-ffast-math>
        )
    endif()

    if(APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        target_compile_options(image_processing PRIVATE 
            $<$<CONFIG:Release>:-mcpu=native>
            $<$<CONFIG:Release>:-mtune=native>
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

# --- OpenMP ---
if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # First check standard Homebrew paths
    if(EXISTS "/opt/homebrew/opt/libomp")
        set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
    elseif(EXISTS "/usr/local/opt/libomp")
        set(OpenMP_ROOT "/usr/local/opt/libomp")
    endif()

    if(OpenMP_ROOT)
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${OpenMP_ROOT})
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${OpenMP_ROOT}/include")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY "${OpenMP_ROOT}/lib/libomp.dylib")
        
        include_directories("${OpenMP_ROOT}/include")
        link_directories("${OpenMP_ROOT}/lib")
    endif()
endif()

find_package(OpenMP REQUIRED)

# --- OpenCV ---
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui dnn) 

# =========================================================
# LOKALE ONNX RUNTIME (Struktur: libs/onnxruntime)
# =========================================================
set(OR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../libs/onnxruntime")
set(ONNXRUNTIME_INCLUDE_DIR "${OR_ROOT}/include")

if(NOT EXISTS "${ONNXRUNTIME_INCLUDE_DIR}/onnxruntime_cxx_api.h")
    message(FATAL_ERROR "❌ ONNX Runtime Headers nicht gefunden in: ${ONNXRUNTIME_INCLUDE_DIR}")
endif()

find_library(ONNXRUNTIME_LIB 
    NAMES onnxruntime
    PATHS "${OR_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(ONNXRUNTIME_LIB)
    message(STATUS "✅ Using Local ONNX Runtime Lib: ${ONNXRUNTIME_LIB}")
else()
    message(FATAL_ERROR "❌ Local ONNX Runtime Lib not found in ${OR_ROOT}/lib")
endif()

# =========================================================
# LOKALE HALIDE LIB (Struktur: libs/Halide)
# =========================================================
set(HALIDE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../libs/Halide")
set(HALIDE_INCLUDE_DIR "${HALIDE_ROOT}/include")

# 1. Header Pfad prüfen
if(NOT EXISTS "${HALIDE_INCLUDE_DIR}/Halide.h")
    message(FATAL_ERROR "❌ Halide Headers nicht gefunden in: ${HALIDE_INCLUDE_DIR}")
endif()

# 2. Library finden
find_library(HALIDE_LIB 
    NAMES Halide 
    PATHS "${HALIDE_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(HALIDE_LIB)
    message(STATUS "✅ Using Local Halide Lib: ${HALIDE_LIB}")
else()
    message(FATAL_ERROR "❌ Local Halide Lib not found in ${HALIDE_ROOT}/lib")
endif()


# =========================================================
# RAW_PROCESSING
# =========================================================

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../raw_processing/CMakeLists.txt")
    message(STATUS "🔗 Einbinden von raw_processing...")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../raw_processing" 
                     "${CMAKE_CURRENT_BINARY_DIR}/raw_processing_build")
else()
    message(FATAL_ERROR "❌ raw_processing nicht gefunden! Erwarteter Pfad: ${CMAKE_CURRENT_SOURCE_DIR}/../raw_processing")
endif()




# ---------------------------------------------------------
# 3. Quellen
# ---------------------------------------------------------
file(GLOB_RECURSE CORE_SOURCES        CONFIGURE_DEPENDS "core/*.cpp")
file(GLOB_RECURSE OPERATION_SOURCES   CONFIGURE_DEPENDS "operations/*.cpp")
file(GLOB_RECURSE CONTROLLER_SOURCES CONFIGURE_DEPENDS "controller/*.cpp")
file(GLOB_RECURSE UTILS_SOURCES       CONFIGURE_DEPENDS "utils/*.cpp")
file(GLOB_RECURSE STYLE_SOURCES       CONFIGURE_DEPENDS "style_transfer/*.cpp")

target_sources(image_processing PRIVATE
    ${CORE_SOURCES}
    ${OPERATION_SOURCES}
    ${CONTROLLER_SOURCES}
    ${UTILS_SOURCES}
    ${STYLE_SOURCES}
)

# ---------------------------------------------------------
# 4. Includes
# ---------------------------------------------------------
target_include_directories(image_processing PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/operations>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/controller>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/style_transfer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mock>
    
    $<INSTALL_INTERFACE:include/image_processing>
)

target_include_directories(image_processing SYSTEM PUBLIC
    # --- Externe Libraries Includes ---
    ${ONNXRUNTIME_INCLUDE_DIR}
    ${HALIDE_INCLUDE_DIR}
)

if(OpenMP_ROOT)
    target_include_directories(image_processing PRIVATE "${OpenMP_ROOT}/include")
endif()

target_compile_features(image_processing PUBLIC cxx_std_20)

# ---------------------------------------------------------
# 5. Linking
# ---------------------------------------------------------
target_link_libraries(image_processing 
    PUBLIC 
        ${OpenCV_LIBS}
        ${ONNXRUNTIME_LIB}
        ${HALIDE_LIB}
        raw_processing
        # System Libraries für Halide JIT auf macOS (Wichtig!)
        $<$<PLATFORM_ID:Darwin>:dl>
        $<$<PLATFORM_ID:Darwin>:pthread>
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(image_processing PRIVATE OpenMP::OpenMP_CXX)
elseif(APPLE AND OpenMP_omp_LIBRARY)
    target_link_libraries(image_processing PRIVATE ${OpenMP_omp_LIBRARY})
    target_compile_options(image_processing PRIVATE "-Xpreprocessor" "-fopenmp")
endif()

# --- CoreML Frameworks linken (Wichtig für M1 Pro!) ---
if(APPLE)
    find_library(COREML_LIB CoreML)
    find_library(FOUNDATION_LIB Foundation)
    
    if(COREML_LIB AND FOUNDATION_LIB)
        message(STATUS "✅ Found Apple CoreML Frameworks")
        target_link_libraries(image_processing PRIVATE ${COREML_LIB} ${FOUNDATION_LIB})
    else()
        message(WARNING "⚠️ Could not find CoreML system frameworks!")
    endif()
endif()

# ---------------------------------------------------------
# 7. Halide Generator (AOT)
# ---------------------------------------------------------

# 1. Build the Generator Executable
add_executable(generate_photo_adjustment "${CMAKE_CURRENT_SOURCE_DIR}/generator/photo_adjustment_generator.cpp")
target_link_libraries(generate_photo_adjustment PRIVATE ${HALIDE_LIB} ${ONNXRUNTIME_LIB} ${OpenCV_LIBS})
target_include_directories(generate_photo_adjustment PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/utils"
    ${HALIDE_INCLUDE_DIR}
    ${ONNXRUNTIME_INCLUDE_DIR}
)
# Helper utils for Generator (reusing existing utils files)
target_sources(generate_photo_adjustment PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/halide_image_utils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/bilateral_filter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/gaussian.cpp"
)

# 2. Run the Generator
# This command runs the generator executable to produce the .a and .h files
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/photo_adjustment.a" "${CMAKE_CURRENT_BINARY_DIR}/photo_adjustment.h"
    COMMAND generate_photo_adjustment
            "-g" "photo_adjustment"
            "-o" "${CMAKE_CURRENT_BINARY_DIR}"
            "-f" "photo_adjustment"
            "-p" "${HALIDE_ROOT}/lib/libautoschedule_adams2019.so"
            target=host-metal
            autoscheduler=Adams2019
    DEPENDS generate_photo_adjustment
    COMMENT "Generating Halide AOT code for photo_adjustment (Auto-Scheduled)..."
)

# 3. Import the Generated Library
add_library(photo_adjustment_aot STATIC IMPORTED)
set_target_properties(photo_adjustment_aot PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/photo_adjustment.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}"
)

# Define AOT Enabler
target_compile_definitions(image_processing PUBLIC AOT_ENABLED)
target_link_libraries(image_processing PRIVATE photo_adjustment_aot)

# Ensure generation happens before linking
add_custom_target(trigger_photo_adjustment_gen DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/photo_adjustment.a")
add_dependencies(image_processing trigger_photo_adjustment_gen)

# ---------------------------------------------------------
# Test & Installation
# ---------------------------------------------------------
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/image_processing_test.cpp")
    add_executable(image_processing_test "${CMAKE_CURRENT_SOURCE_DIR}/image_processing_test.cpp")
    target_link_libraries(image_processing_test PRIVATE 
        image_processing 
        photo_adjustment_aot
        "-framework Metal"
        "-framework Foundation"
    )
    
    if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
         target_compile_options(image_processing_test PRIVATE "-Xpreprocessor" "-fopenmp")
    endif()

    # Output-Pfad festlegen (damit es in build/bin landet)
    set_target_properties(image_processing_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_data")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../AI_models")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../AI_models" DESTINATION "${CMAKE_BINARY_DIR}/bin")
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../images")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../images" DESTINATION "${CMAKE_BINARY_DIR}/bin")
    endif()
endif()

install(TARGETS image_processing
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY core/ operations/ utils/ style_transfer/
    DESTINATION include/image_processing
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# ---------------------------------------------------------
# STATUS CHECK
# ---------------------------------------------------------
message(STATUS "---------------------------------------------------")
message(STATUS ">> BUILD CONFIGURATION CHECK <<")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "PERFORMANCE: ✅  OPTIMIZATIONS ON (-O3 active)")
else()
    message(STATUS "PERFORMANCE: ❌  SLOW MODE DETECTED (Debug active)")
endif()
message(STATUS "ONNX Runtime: ✅  Local Lib (${ONNXRUNTIME_LIB})")
message(STATUS "Halide:       ✅  Local Lib (${HALIDE_LIB})")
message(STATUS "---------------------------------------------------")