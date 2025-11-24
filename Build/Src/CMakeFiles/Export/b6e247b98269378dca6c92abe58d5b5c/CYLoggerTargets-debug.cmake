#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "CYLogger::CYLogger" for configuration "Debug"
set_property(TARGET CYLogger::CYLogger APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(CYLogger::CYLogger PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/CYLogger.lib"
  )

list(APPEND _cmake_import_check_targets CYLogger::CYLogger )
list(APPEND _cmake_import_check_files_for_CYLogger::CYLogger "${_IMPORT_PREFIX}/lib/CYLogger.lib" )

# Import target "CYLogger::CYCoroutine_static" for configuration "Debug"
set_property(TARGET CYLogger::CYCoroutine_static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(CYLogger::CYCoroutine_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/CYCoroutine.lib"
  )

list(APPEND _cmake_import_check_targets CYLogger::CYCoroutine_static )
list(APPEND _cmake_import_check_files_for_CYLogger::CYCoroutine_static "${_IMPORT_PREFIX}/lib/CYCoroutine.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
