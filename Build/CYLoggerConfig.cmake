
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was CYLoggerConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

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

include(CMakeFindDependencyMacro)

# Locate dependencies provided by the host project
find_dependency(Threads REQUIRED)

# Import the generated targets file
include("${CMAKE_CURRENT_LIST_DIR}/CYLoggerTargets.cmake")

# Publish helper variables for consumers
set(CYLOGGER_VERSION 1.0.0)
set(CYLOGGER_FOUND TRUE)

# Validate requested components
check_required_components(CYLogger)

# Provide convenience aliases for callers
if(TARGET CYLogger::CYLogger)
    set(CYLOGGER_LIBRARIES CYLogger::CYLogger)
    get_target_property(CYLOGGER_INCLUDE_DIRS CYLogger::CYLogger INTERFACE_INCLUDE_DIRECTORIES)
endif()
