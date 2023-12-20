include(cmake/CPM.cmake)
set(CPM_USE_LOCAL_PACKAGES OFF)
set(CPM_SOURCE_CACHE=$HOME/.cache/CPM)

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(cloysterhpc_setup_dependencies)

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if(NOT (TARGET Boost::headers OR TARGET Boost::system OR TARGET Boost::thread))
    if (cloysterhpc_ENABLE_CONAN)
      CPMFindPackage(NAME Boost)
    else()
      CPMAddPackage(
        NAME Boost
        VERSION 1.82.0
        GITHUB_REPOSITORY "boostorg/boost"
        GIT_TAG "boost-1.82.0"
      )
    endif()
  endif()
  
  if(NOT TARGET fmtlib::fmtlib)
    if (cloysterhpc_ENABLE_CONAN)
      CPMFindPackage(NAME fmt)
    else()
      CPMAddPackage("gh:fmtlib/fmt#9.1.0")
    endif()
  endif()

  if(NOT TARGET spdlog::spdlog)
    if (cloysterhpc_ENABLE_CONAN)
      CPMFindPackage(NAME spdlog
       OPTIONS
        "SPDLOG_FMT_EXTERNAL ON")
    else()
      CPMAddPackage(
        NAME
        spdlog
        VERSION
        1.11.0
        GITHUB_REPOSITORY
        "gabime/spdlog"
        OPTIONS
        "SPDLOG_FMT_EXTERNAL ON")
    endif()
  endif()

  if(NOT TARGET magic_enum::magic_enum)
    if (cloysterhpc_ENABLE_CONAN)
      CPMFindPackage(NAME magic_enum)
    else()
      CPMAddPackage(
        NAME magic_enum
        VERSION 0.9.3
        GITHUB_REPOSITORY Neargye/magic_enum
        #OPTIONS "MAGIC_ENUM_OPT_INSTALL YES"
      )
    endif()
  endif()

  if(NOT TARGET gsl-lite::gsl-lite)
    if (cloysterhpc_ENABLE_CONAN)
      CPMFindPackage(NAME gsl-lite)
    else()
      CPMAddPackage("gh:gsl-lite/gsl-lite@0.41.0")
    endif()
  endif()

  if(NOT TARGET CLI11::CLI11)
    if (cloysterhpc_ENABLE_CONAN)
      CPMFindPackage(NAME CLI11)
    else()
      CPMAddPackage("gh:CLIUtils/CLI11@2.3.2")
    endif()
  endif()

  if(NOT TARGET doctest::doctest)
    if (cloysterhpc_ENABLE_CONAN)
      CPMFindPackage(NAME doctest)
    else()
      CPMAddPackage("gh:doctest/doctest@2.4.11")
    endif()
  endif()

  if(NOT TARGET cryptopp::cryptopp)
    if (cloysterhpc_ENABLE_CONAN)
      CPMFindPackage(NAME cryptopp)
    else()
      CPMAddPackage("gh:cryptopp/cryptopp@8.7.0")
    endif()
  endif()

  if(NOT TARGET SDBusCpp::sdbus-c++)
    if (cloysterhpc_ENABLE_CONAN)
      CPMFindPackage(NAME sdbus-c++)
    else()
      CPMAddPackage(
        NAME
        sdbus-c++
        VERSION
        2.0.0
        GITHUB_REPOSITORY
        "Kistler-Groups/sdbus-cpp")
    endif()
  endif()

  if(NOT TARGET hwinfo::HWinfo)
    CPMAddPackage(
      NAME hwinfo
      GITHUB_REPOSITORY lfreist/hwinfo
      GIT_TAG main
      OPTIONS "NO_OCL ON"
    )
  endif()

  # Packages only available with CPM
  #if(NOT TARGET tools::tools)
  #  CPMAddPackage("gh:lefticus/tools#update_build_system")
  #endif()

  if(NOT TARGET SimpleIni::SimpleIni)
    CPMAddPackage("gh:brofield/simpleini@4.20")
  endif()

  # Standalone packages
  # Include module path for packages that we need to find or build
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
  include(cmake/Findnewt.cmake)
  if(NOT TARGET newt)
    CPMFindPackage(NAME newt)
  endif()

  # Set the variable ${STDC++FS} to the correct library
  include(cmake/Addstdcppfs.cmake)

endfunction()
