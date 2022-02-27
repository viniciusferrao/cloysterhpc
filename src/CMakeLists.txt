# Override DEBUG CXX flags
set(CUSTOM_DEBUG_FLAGS "-D_DEBUG_")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CUSTOM_DEBUG_FLAGS}")

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
find_package(fmt REQUIRED)
include_directories(${fmt_INCLUDE_DIRS})

# Library: Boost
find_package(Boost REQUIRED
    COMPONENTS system thread
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if(${os_version_suffix} MATCHES "el8")
        if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            link_directories(/opt/rh/gcc-toolset-11/root/usr/lib/gcc/x86_64-redhat-linux/11)
        endif()
        set(STDC++FS "stdc++fs")
    endif()
endif()

# Library: spdlog
find_package(spdlog)

# Generate a dummy binary
# TODO: Create a custom build target
option(DUMMY "Create a dummy executable" ON)
function(configureProject)
    if(DUMMY)
        target_compile_options(project_options INTERFACE -D_DUMMY_ -D_DEBUG_)
    endif()
endfunction()
configureProject()

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

target_link_libraries(main
    PRIVATE project_options
            project_warnings

    ${newt_LIBRARIES}
    Boost::headers Boost::system Boost::thread
    fmt::fmt
    spdlog::spdlog
    resolv
    ${STDC++FS}
)

# Set target locations to the root folder of the project in /bin and /lib.
set_target_properties(main
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)
