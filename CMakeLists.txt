cmake_minimum_required(VERSION 3.21)

project(CloysterHPC
        DESCRIPTION "Cloyster HPC is a turnkey HPC cluster solution with an user-friendly installer"
        HOMEPAGE_URL "https://github.com/viniciusferrao/cloysterhpc"
        LANGUAGES CXX
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS                "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -D_DEBUG_")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os")
set(CMAKE_CXX_FLAGS_RELEASE        "-O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

include(cmake/disableBuildsOnSourceDir.cmake)
include(cmake/cache.cmake)

#add_subdirectory(src)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
add_executable(main ${SOURCE_FILES})

option(DUMMY "Create a dummy executable" ON)
option(DEBUG "Create a debug executable" ON)

function(configureProject)
    if (DUMMY)
        add_compile_options(-D_DUMMY_)
    endif()
endfunction()

configureProject()

# newt
if (LibNewt_LIBRARIES AND LibNewt_INCLUDE_DIRS)
    set (LibNewt_FIND_QUIETLY TRUE)
endif (LibNewt_LIBRARIES AND LibNewt_INCLUDE_DIRS)

find_path (LibNewt_INCLUDE_DIRS NAMES newt.h)
find_library (LibNewt_LIBRARIES NAMES newt)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibNewt DEFAULT_MSG
        LibNewt_LIBRARIES
        LibNewt_INCLUDE_DIRS)

mark_as_advanced(LibNewt_INCLUDE_DIRS LibNewt_LIBRARIES)

#find_package(LibNewt REQUIRED)
# end of newt

# fmt
find_package(fmt REQUIRED)
include_directories(${fmt_INCLUDE_DIRS})

# Boost
find_package(Boost REQUIRED
        COMPONENTS system thread log log_setup)

# -l flags for linking prog target
#include_directories(${LibNewt_INCLUDE_DIRS})
target_link_libraries(main
        ${LibNewt_LIBRARIES}
        Boost::headers Boost::system Boost::thread Boost::log Boost::log_setup
        fmt::fmt
)

# Add this compile definition for debug builds, this same logic works for
# target_compile_options, target_link_options, etc.
target_compile_definitions(main PRIVATE
    $<$<CONFIG:Debug>:
        _DEBUG_=1
    >
)
