#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cppcoro::cppcoro" for configuration ""
set_property(TARGET cppcoro::cppcoro APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(cppcoro::cppcoro PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libcppcoro.a"
  )

list(APPEND _cmake_import_check_targets cppcoro::cppcoro )
list(APPEND _cmake_import_check_files_for_cppcoro::cppcoro "${_IMPORT_PREFIX}/lib/libcppcoro.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
