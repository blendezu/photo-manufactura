#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Halide::Halide" for configuration "Release"
set_property(TARGET Halide::Halide APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Halide::Halide PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libHalide.21.0.0.dylib"
  IMPORTED_SONAME_RELEASE "@rpath/libHalide.21.dylib"
  )

list(APPEND _cmake_import_check_targets Halide::Halide )
list(APPEND _cmake_import_check_files_for_Halide::Halide "${_IMPORT_PREFIX}/lib/libHalide.21.0.0.dylib" )

# Import target "Halide::GenGen" for configuration "Release"
set_property(TARGET Halide::GenGen APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Halide::GenGen PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libHalide_GenGen.a"
  )

list(APPEND _cmake_import_check_targets Halide::GenGen )
list(APPEND _cmake_import_check_files_for_Halide::GenGen "${_IMPORT_PREFIX}/lib/libHalide_GenGen.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
