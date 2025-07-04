cmake_minimum_required(VERSION 3.26.5)

# It would be hard to remove conan because cmake does a

# This template attempts to be "fetch_content"-able
# so that it works well with tools like CPM or other
# manual dependency management

# Load PkgConfig so we can use pkg_check_modules later
# to find local packages.
find_package(PkgConfig REQUIRED)

# Only set the cxx_standard if it is not set by someone else
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif ()

# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Conan 2.0 support
option(cloysterhpc_ENABLE_CONAN "Use Conan 2 to manage dependencies" ON)
if (cloysterhpc_ENABLE_CONAN)
    set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES cmake/conan_provider.cmake)
endif ()

# Set a default build type if none was specified
# This is required for Conan 2.0 to work properly
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE
            Debug
            CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui, ccmake
    set_property(
            CACHE CMAKE_BUILD_TYPE
            PROPERTY STRINGS
            "Debug"
            "Release"
            "MinSizeRel"
            "RelWithDebInfo")
endif ()

# Set the project name and language
project(
        CloysterHPC
        VERSION 0.1.1
        DESCRIPTION "Cloyster HPC is a turnkey HPC cluster solution with an user-friendly installer"
        HOMEPAGE_URL "https://github.com/viniciusferrao/cloysterhpc"
        LANGUAGES CXX C)

include(cmake/PreventInSourceBuilds.cmake)
include(ProjectOptions.cmake)


cloysterhpc_setup_options()

cloysterhpc_global_options()
include(Dependencies.cmake)
cloysterhpc_setup_dependencies()

cloysterhpc_local_options()

# don't know if this should be set globally from here or not...
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Get the GIT SHA of the current commit
execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Check if GIT_SHA is empty and set to "Unknown" if it is
if ("${GIT_SHA}" STREQUAL "")
    set(GIT_SHA "Unknown")
endif ()

# Get the short SHA (first 8 characters) if GIT_SHA is not "Unknown"
if (NOT "${GIT_SHA}" STREQUAL "Unknown")
    string(SUBSTRING "${GIT_SHA}" 0 8 GIT_SHORT_SHA)
else ()
    set(GIT_SHORT_SHA "Unknown")
endif ()

# Cache the variables for future use
set(GIT_SHA "${GIT_SHA}" CACHE STRING "SHA this build was generated from")
set(GIT_SHORT_SHA "${GIT_SHORT_SHA}" CACHE STRING "Short SHA this build was generated from")

set(cloysterhpc_BINARY_NAME "cloysterhpc")

target_compile_features(cloysterhpc_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})

add_library(cloysterhpc::cloysterhpc_options ALIAS cloysterhpc_options)
add_library(cloysterhpc::cloysterhpc_warnings ALIAS cloysterhpc_warnings)

# configure files based on CMake configuration options
add_subdirectory(configured_files)

# Adding the src:
add_subdirectory(src)

# Don't even look at tests if we're not top level
if (NOT PROJECT_IS_TOP_LEVEL)
    return()
endif ()

# Adding the tests:
include(CTest)

if (BUILD_TESTING)
    add_subdirectory(test)
endif ()


if (cloysterhpc_BUILD_FUZZ_TESTS)
    message(AUTHOR_WARNING "Building Fuzz Tests, using fuzzing sanitizer https://www.llvm.org/docs/LibFuzzer.html")
    if (NOT cloysterhpc_ENABLE_ADDRESS_SANITIZER AND NOT cloysterhpc_ENABLE_THREAD_SANITIZER)
        message(WARNING "You need asan or tsan enabled for meaningful fuzz testing")
    endif ()
    add_subdirectory(fuzz_test)

endif ()

# If MSVC is being used, and ASAN is enabled, we need to set the debugger environment
# so that it behaves well with MSVC's debugger, and we can run the target from visual studio
if (MSVC)
    get_all_installable_targets(all_targets)
    message("all_targets=${all_targets}")
    set_target_properties(${all_targets} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
endif ()

# set the startup project for the "play" button in MSVC
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${cloysterhpc_BINARY_NAME})

if (CMAKE_SKIP_INSTALL_RULES)
    return()
endif ()

include(cmake/PackageProject.cmake)

# Add other targets that you want installed here, by default we just package the one executable
# we know we want to ship
cloysterhpc_package_project(
        TARGETS
        ${cloysterhpc_BINARY_NAME}
        cloysterhpc_options
        cloysterhpc_warnings
        # FIXME: this does not work! CK
        # PRIVATE_DEPENDENCIES_CONFIGURED project_options project_warnings
)
