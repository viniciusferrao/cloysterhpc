# Add source files and set linking
file(GLOB SOURCE_FILES
        *.cpp
        mailsystem/*.cpp
        presenter/*.cpp
        queuesystem/*.cpp
        services/*.cpp
        view/*.cpp)
message(STATUS "Source files found are: ${SOURCE_FILES}")

add_executable(main ${SOURCE_FILES})

target_link_libraries(
  main
  PRIVATE
    cloysterhpc::cloysterhpc_options
    cloysterhpc::cloysterhpc_warnings)

target_link_system_libraries(
  main
  PRIVATE
    ${NEWT_LIBRARY}
    fmt::fmt
    CLI11::CLI11
    Boost::headers
    Boost::system
    Boost::thread
    spdlog::spdlog
    gsl::gsl-lite
    magic_enum::magic_enum
    SimpleIni::SimpleIni
    resolv
    ${STDC++FS}
    doctest::doctest)

target_include_directories(main PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
target_include_directories(main ${WARNING_GUARD} PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

# Set target locations to the root folder of the project in /bin and /lib.
# WARNING: This will break multiple target compile, disable it if necessary
# This is only for convenience
#set_target_properties(main
#    PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
#    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
#    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
#)
