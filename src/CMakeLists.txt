# Add all source files except main.cpp
file(GLOB_RECURSE SOURCE_FILES *.cpp)
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

message(STATUS "Source files found are: ${SOURCE_FILES}")

# Build a library and a main binary that links to the library
add_library(cloysterhpc_object OBJECT ${SOURCE_FILES})
add_library(cloysterhpc_static STATIC $<TARGET_OBJECTS:cloysterhpc_object>)
add_executable(main main.cpp)

target_link_libraries(
  main
  PRIVATE
    cloysterhpc_static
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
    doctest::doctest
    cryptopp::cryptopp)

target_link_libraries(
  cloysterhpc_object
  PRIVATE
    cloysterhpc::cloysterhpc_options
    cloysterhpc::cloysterhpc_warnings)

target_link_system_libraries(
  cloysterhpc_object
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
    doctest::doctest
    cryptopp::cryptopp)

target_include_directories(cloysterhpc_static PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
target_include_directories(cloysterhpc_static ${WARNING_GUARD} PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_include_directories(cloysterhpc_object PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
target_include_directories(cloysterhpc_object ${WARNING_GUARD} PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

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
