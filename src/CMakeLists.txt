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

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  if(${os_version_suffix} MATCHES "el")
    set(STDC++FS "stdc++fs")
  endif()
endif()

# Dummy binary
option(DUMMY "Create a dummy executable" ON)
function(configureProject)
    if(DUMMY)
      target_compile_options(project_options INTERFACE -D_DUMMY_ -D_DEBUG_)
    endif()
endfunction()
configureProject()

# -l flags for linking prog target
#include_directories(${LibNewt_INCLUDE_DIRS})
file(GLOB_RECURSE SOURCE_FILES *.cpp)
add_executable(main ${SOURCE_FILES})
target_link_libraries(main
        PRIVATE project_options
                project_warnings

        ${LibNewt_LIBRARIES}
        Boost::headers Boost::system Boost::thread Boost::log Boost::log_setup
        fmt::fmt
        ${STDC++FS})

#find_package(fmt)
#find_package(spdlog)
#find_package(docopt)
#
## Generic test that uses conan libs
#add_executable(intro main.cpp)
#target_link_libraries(
#  intro
#  PRIVATE project_options
#          project_warnings
#          docopt::docopt
#          fmt::fmt
#          spdlog::spdlog)
