# Library: newt
find_package(newt REQUIRED)

# Library: fmt
find_package(fmt CONFIG REQUIRED)

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
  PRIVATE ${NEWT_LIBRARY} fmt::fmt CLI11::CLI11 Boost::headers Boost::system Boost::thread spdlog::spdlog gsl::gsl-lite magic_enum::magic_enum resolv ${STDC++FS})

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
