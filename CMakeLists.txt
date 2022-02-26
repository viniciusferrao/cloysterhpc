# CMake infrastructure heavily based on:
# https://github.com/cpp-best-practices/cpp_starter_project

cmake_minimum_required(VERSION 3.18.2)

project(CloysterHPC
        VERSION 0.1
        DESCRIPTION "Cloyster HPC is a turnkey HPC cluster solution with an user-friendly installer"
        HOMEPAGE_URL "https://github.com/viniciusferrao/cloysterhpc"
        LANGUAGES CXX
)

# Enterprise Linux (EL) detection code, we may find el2 to el9 kernels.
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    string (REGEX MATCH "\\el[2-9]" os_version_suffix ${CMAKE_SYSTEM})
endif()

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

# There is a bug on CMake that using cxx_std_20 always defaults to --std=gnu++20
# instead of --std=c++20:
# https://gitlab.kitware.com/cmake/cmake/-/issues/22224
# https://stackoverflow.com/questions/67641124/gcc-11-how-to-tell-cmake-i-dont-want-the-default-c-gnu-extensions
# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if(${os_version_suffix} MATCHES "el8")
        # If we match Enterprise Linux 8 now compiles with C++20, but we need to
        # provide gcc-toolset-11 to make it work. C++17 is too "old" now.
        target_compile_features(project_options INTERFACE cxx_std_20)
    endif()
else()
    target_compile_features(project_options INTERFACE cxx_std_20)
endif()
# TODO: Find a way to set those features as project_options
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
    if(ENABLE_BUILD_WITH_TIME_TRACE)
        target_compile_options(project_options INTERFACE -ftime-trace)
    endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# Add linker configuration
include(cmake/Linker.cmake)
configure_linker(project_options)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# enable doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_PCH "Enable Precompiled Headers" OFF)
option(ENABLE_CONAN "Use Conan for dependency management" ON)
option(ENABLE_TESTING "Enable Test Builds" OFF)
option(ENABLE_FUZZING "Enable Fuzzing Builds" OFF)
option(ENABLE_UNITY "Enable Unity builds of projects" OFF)

# Very basic PCH example
if(ENABLE_PCH)
    # This sets a global PCH parameter, each project will build its own PCH, which is a good idea if any #define's change
    #
    # consider breaking this out per project as necessary
    target_precompile_headers(
            project_options
            INTERFACE
            <vector>
            <string>
            <map>
            <utility>)
endif()

if(ENABLE_CONAN)
    include(cmake/Conan.cmake)
    run_conan()
endif()

if(ENABLE_TESTING)
    enable_testing()
    message("Building Tests. Be sure to check out test/constexpr_tests for constexpr testing")
    add_subdirectory(test)
endif()

if(ENABLE_FUZZING)
    message("Building Fuzz Tests, using fuzzing sanitizer https://www.llvm.org/docs/LibFuzzer.html")
    add_subdirectory(fuzz_test)
endif()

if(ENABLE_UNITY)
    # Add for any project you want to apply unity builds for
    set_target_properties(project_options PROPERTIES UNITY_BUILD ON)
endif()

add_subdirectory(src)
