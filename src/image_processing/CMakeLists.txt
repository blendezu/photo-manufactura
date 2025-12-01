cmake_minimum_required(VERSION 3.20)
project(image_processing LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==========================================
# FIX 1: Build-Type auf Release erzwingen
# ==========================================
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

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
# WICHTIG: Erst Pfade setzen, DANN suchen!
if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
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

# Jetzt erst suchen, da die Pfade bekannt sind
find_package(OpenMP REQUIRED)


# --- OpenCV ---
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui dnn) 

# --- Google Highway ---
# list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
# find_package(hwy REQUIRED)

# =========================================================
# ⚠️ NEU: LOKALE ONNX RUNTIME (Kein Homebrew mehr!)
# =========================================================

# Pfad zu deinem lokalen libs Ordner (Relativ zum Projekt-Root)
set(OR_ROOT "${CMAKE_SOURCE_DIR}/libs/onnxruntime")

# 1. Header Pfad setzen
set(ONNXRUNTIME_INCLUDE_DIR "${OR_ROOT}/include")

# Sicherheitscheck: Haben wir den Ordner wirklich gefunden?
if(NOT EXISTS "${ONNXRUNTIME_INCLUDE_DIR}/onnxruntime_cxx_api.h")
    message(FATAL_ERROR "❌ ONNX Runtime Headers nicht gefunden in: ${ONNXRUNTIME_INCLUDE_DIR}.\nBitte prüfe, ob der Ordner 'libs/onnxruntime/include' existiert!")
endif()

# 2. Library finden (Nur im lokalen Ordner suchen!)
find_library(ONNXRUNTIME_LIB 
    NAMES onnxruntime
    PATHS "${OR_ROOT}/lib"
    NO_DEFAULT_PATH  # WICHTIG: Verhindert Suche in /usr/local oder Homebrew
)

if(ONNXRUNTIME_LIB)
    message(STATUS "✅ Using Local ONNX Runtime Lib: ${ONNXRUNTIME_LIB}")
else()
    message(FATAL_ERROR "❌ Local ONNX Runtime Lib not found in ${OR_ROOT}/lib")
endif()
# =========================================================


# ---------------------------------------------------------
# 3. Quellen
# ---------------------------------------------------------
file(GLOB_RECURSE CORE_SOURCES       CONFIGURE_DEPENDS "core/*.cpp")
file(GLOB_RECURSE OPERATION_SOURCES  CONFIGURE_DEPENDS "operations/*.cpp")
file(GLOB_RECURSE UTILS_SOURCES      CONFIGURE_DEPENDS "utils/*.cpp")
file(GLOB_RECURSE STYLE_SOURCES      CONFIGURE_DEPENDS "style_transfer/*.cpp")

target_sources(image_processing PRIVATE
    ${CORE_SOURCES}
    ${OPERATION_SOURCES}
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
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/style_transfer>
    
    # --- NEU: Lokale ONNX Runtime Headers ---
    ${ONNXRUNTIME_INCLUDE_DIR}
    
    $<INSTALL_INTERFACE:include/image_processing>
)

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
        ${ONNXRUNTIME_LIB} # Linkt gegen deine lokale Datei
        # hwy::hwy
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(image_processing PRIVATE OpenMP::OpenMP_CXX)
elseif(APPLE AND OpenMP_omp_LIBRARY)
    target_link_libraries(image_processing PRIVATE ${OpenMP_omp_LIBRARY})
    target_compile_options(image_processing PRIVATE "-Xpreprocessor" "-fopenmp")
endif()

# --- NEU: CoreML Frameworks linken (Wichtig für M1 Pro!) ---
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
# 6. Test & Installation
# ---------------------------------------------------------
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp")
    add_executable(image_processing_test "${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp")
    target_link_libraries(image_processing_test PRIVATE image_processing)
    
    if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
         target_compile_options(image_processing_test PRIVATE "-Xpreprocessor" "-fopenmp")
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_data")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/models")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/models" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
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
message(STATUS "---------------------------------------------------")