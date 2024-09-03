function(cloysterhpc_package_project)
  cmake_policy(SET CMP0103 NEW) # disallow multiple calls with the same NAME

  set(_options ARCH_INDEPENDENT # default to false
  )
  set(_oneValueArgs
      # default to the project_name:
      NAME
      COMPONENT
      # default to project version:
      VERSION
      # default to semver
      COMPATIBILITY
      # default to ${CMAKE_BINARY_DIR}
      CONFIG_EXPORT_DESTINATION
      # default to ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${NAME} suitable for vcpkg, etc.
      CONFIG_INSTALL_DESTINATION)
  set(_multiValueArgs
      # recursively found for the current folder if not specified
      TARGETS
      # a list of public/interface include directories or files
      PUBLIC_INCLUDES
      # the names of the INTERFACE/PUBLIC dependencies that are found using `CONFIG`
      PUBLIC_DEPENDENCIES_CONFIGURED
      # the INTERFACE/PUBLIC dependencies that are found by any means using `find_dependency`.
      # the arguments must be specified within double quotes (e.g. "<dependency> 1.0.0 EXACT" or "<dependency> CONFIG").
      PUBLIC_DEPENDENCIES
      # the names of the PRIVATE dependencies that are found using `CONFIG`. Only included when BUILD_SHARED_LIBS is OFF.
      PRIVATE_DEPENDENCIES_CONFIGURED
      # PRIVATE dependencies that are only included when BUILD_SHARED_LIBS is OFF
      PRIVATE_DEPENDENCIES)

  cmake_parse_arguments(
    _PackageProject
    "${_options}"
    "${_oneValueArgs}"
    "${_multiValueArgs}"
    "${ARGN}")

  # Set default options
  include(GNUInstallDirs) # Define GNU standard installation directories such as CMAKE_INSTALL_DATADIR

  # set default packaged targets
  if(NOT _PackageProject_TARGETS)
    get_all_installable_targets(_PackageProject_TARGETS)
    message(STATUS "package_project: considering ${_PackageProject_TARGETS} as the exported targets")
  endif()

  # default to the name of the project or the given name
  if("${_PackageProject_NAME}" STREQUAL "")
    set(_PackageProject_NAME ${PROJECT_NAME})
  endif()

  set(_PackageProject_EXPORT ${_PackageProject_NAME})

  # Installation of package (compatible with vcpkg, etc)
  set(CMAKE_INSTALL_PREFIX "/opt/${CMAKE_PROJECT_NAME}")

  install(
    TARGETS ${_PackageProject_TARGETS}
    EXPORT ${_PackageProject_EXPORT}
    LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" COMPONENT shlib
    ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" COMPONENT lib
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/sbin" COMPONENT bin
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${_PackageProject_NAME}" COMPONENT dev)

  include(cmake/CPackProperties.cmake)
endfunction()
