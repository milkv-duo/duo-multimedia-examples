#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Poco::MongoDB" for configuration "Release"
set_property(TARGET Poco::MongoDB APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Poco::MongoDB PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libPocoMongoDB.a"
  )

list(APPEND _cmake_import_check_targets Poco::MongoDB )
list(APPEND _cmake_import_check_files_for_Poco::MongoDB "${_IMPORT_PREFIX}/lib/libPocoMongoDB.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
