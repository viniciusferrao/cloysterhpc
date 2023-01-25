# Library: newt
if (newt_LIBRARIES AND newt_INCLUDE_DIRS)
    set (newt_FIND_QUIETLY TRUE)
endif (newt_LIBRARIES AND newt_INCLUDE_DIRS)

find_path (newt_INCLUDE_DIRS NAMES newt.h)
find_library (newt_LIBRARIES NAMES newt)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(newt DEFAULT_MSG
    newt_LIBRARIES
    newt_INCLUDE_DIRS
)

mark_as_advanced(newt_INCLUDE_DIRS newt_LIBRARIES)
include_directories(${newt_INCLUDE_DIRS})

# Library: fmt
find_package(fmt CONFIG REQUIRED)
#include_directories(${fmt_INCLUDE_DIRS})

# Library: CLI11
find_package(CLI11 CONFIG REQUIRED)

# Library: Boost
find_package(Boost CONFIG REQUIRED
    COMPONENTS system thread
)

# Library: spdlog
find_package(spdlog CONFIG REQUIRED)

# Library: gsl-lite
find_package(gsl-lite CONFIG REQUIRED)

# Library: magic_enum
find_package(magic_enum CONFIG REQUIRED)

# Add stdc++fs to EL8 systems with gcc-toolset-12
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if(${os_version_suffix} MATCHES "el8")
        if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            link_directories(/opt/rh/gcc-toolset-12/root/usr/lib/gcc/x86_64-redhat-linux/12)
        endif()
        set(STDC++FS "stdc++fs")
    endif()
endif()

# Add source files and set linking
file(GLOB SOURCE_FILES
        *.cpp
        mailsystem/*.cpp
        presenter/*.cpp
        queuesystem/*.cpp
        services/*.cpp
        view/*.cpp)
message(STATUS "Source files found are ${SOURCE_FILES}")
add_executable(main ${SOURCE_FILES})

target_link_libraries(
  main
  PUBLIC project_options project_warnings
  PRIVATE ${newt_LIBRARIES} fmt::fmt CLI11::CLI11 Boost::headers Boost::system Boost::thread spdlog::spdlog gsl::gsl-lite magic_enum::magic_enum resolv ${STDC++FS})

target_include_directories(main PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

# Set target locations to the root folder of the project in /bin and /lib.
# WARNING: This will break multiple target compile, disable it if necessary
# This is only for convenience
set_target_properties(main
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

# Default config
#find_package(fmt CONFIG REQUIRED)
#find_package(spdlog CONFIG REQUIRED)
#find_package(CLI11 CONFIG REQUIRED)

# Generic test that uses conan libs
#add_executable(intro main.cpp)
#target_link_libraries(
#  intro
#  PUBLIC project_options project_warnings
#  PRIVATE CLI11::CLI11 fmt::fmt spdlog::spdlog)
#
#target_include_directories(intro PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

