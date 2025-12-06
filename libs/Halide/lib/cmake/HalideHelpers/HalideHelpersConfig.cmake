cmake_minimum_required(VERSION 3.28)

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was HalideHelpersConfig.cmake                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set(Halide_HOST_TARGET arm-64-osx)

include(${CMAKE_CURRENT_LIST_DIR}/Halide-Interfaces.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/HalideTargetHelpers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/HalideGeneratorHelpers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/TargetExportScript.cmake)

check_required_components(${CMAKE_FIND_PACKAGE_NAME})
