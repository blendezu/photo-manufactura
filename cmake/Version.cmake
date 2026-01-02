# Photo Manufactura - Centralized Version Management
# This file is the single source of truth for the project version.
# Update version here, and it will propagate to all build scripts and CMake.

set(PHOTO_MANUFACTURA_VERSION_MAJOR 0)
set(PHOTO_MANUFACTURA_VERSION_MINOR 1)
set(PHOTO_MANUFACTURA_VERSION_PATCH 0)

# Computed full version string
set(PHOTO_MANUFACTURA_VERSION 
    "${PHOTO_MANUFACTURA_VERSION_MAJOR}.${PHOTO_MANUFACTURA_VERSION_MINOR}.${PHOTO_MANUFACTURA_VERSION_PATCH}")

# Export for use in shell scripts (written to build directory)
function(write_version_file)
    file(WRITE "${CMAKE_BINARY_DIR}/version.txt" "${PHOTO_MANUFACTURA_VERSION}")
endfunction()
