cmake_minimum_required(VERSION 3.26.5)

project(CloysterHPC-tests LANGUAGES CXX)

# ---- Test as standalone project the exported config package ----
if (PROJECT_IS_TOP_LEVEL OR TEST_INSTALLED_VERSION)
    enable_testing()

    find_package(cloysterhpc CONFIG REQUIRED) # for intro, project_options, ...

    if (NOT TARGET cloysterhpc_options)
        message(FATAL_ERROR "Required config package not found!")
        return() # be strictly paranoid for Template Janitor github action! CK
    endif ()
endif ()

# Provide a simple smoke test to make sure that the CLI works and can display a --help message
# add_test(NAME cli.has_help COMMAND main --help)

# Provide a test to verify that the version being reported from the application
# matches the version given to CMake. This will be important once you package
# your program. Real world shows that this is the kind of simple mistake that is easy
# to make, but also easy to test for.
# add_test(NAME cli.version_matches COMMAND main --version)
# set_tests_properties(cli.version_matches PROPERTIES PASS_REGULAR_EXPRESSION "${PROJECT_VERSION}")

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
message(STATUS "Dedicated test files found are: ${TEST_SOURCES}")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp)
message(STATUS "Source files found are: ${SOURCES}")

add_library(cloysterhpc_test_object OBJECT ${TEST_SOURCES} ${SOURCES})
add_executable(${PROJECT_NAME} main.cpp)

# Enable BUILD_TESTING macro to compile the test shipped on the source files
target_compile_definitions(cloysterhpc_test_object PRIVATE BUILD_TESTING=1)

include(../cmake/CommonLibraries.cmake)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        cloysterhpc_test_object
        cloysterhpc::cloysterhpc_warnings
        cloysterhpc::cloysterhpc_options)

target_link_system_libraries(
        ${PROJECT_NAME}
        PRIVATE
        ${COMMON_LIBS})

# Include glibmm headers as SYSTEM header to avoid warnings
target_include_directories(cloysterhpc_test_object SYSTEM PRIVATE ${GLIBMM_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} ${WARNING_GUARD} PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/../include>)

target_link_libraries(
        cloysterhpc_test_object
        PRIVATE
        cloysterhpc::cloysterhpc_options
        cloysterhpc::cloysterhpc_warnings)

target_link_system_libraries(
        cloysterhpc_test_object
        PRIVATE
        ${COMMON_LIBS})

target_include_directories(cloysterhpc_test_object PRIVATE
        "${CMAKE_BINARY_DIR}/../configured_files/include")
target_include_directories(cloysterhpc_test_object ${WARNING_GUARD} PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/../include>)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD})

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
            TARGET ${PROJECT_NAME}
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
            COMMAND_EXPAND_LISTS)
endif ()

# ---- Add Tests ----

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME ${PROJECT_NAME} COMMAND
# ${PROJECT_NAME})

# if (cloysterhpc_ENABLE_CONAN)
#     string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
#     string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
#     string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CMAKE_SYSTEM_PROCESSOR_LOWER)
# 
#     # Hack to fix wrong architecture name for aarch64 during Conan install
#     if (CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL "aarch64")
#         set(CMAKE_SYSTEM_PROCESSOR_LOWER "armv8")
#     endif ()
# 
#     include(${doctest_DIR}/doctest-${CMAKE_BUILD_TYPE_LOWER}-${CMAKE_SYSTEM_PROCESSOR_LOWER}-data.cmake)
#     include(${doctest_BUILD_DIRS_${CMAKE_BUILD_TYPE_UPPER}}/doctest.cmake)
# else ()
#     include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
# endif ()
# 
# doctest_discover_tests(${PROJECT_NAME})

# ---- code coverage ----

if (ENABLE_TEST_COVERAGE)
    target_compile_options(${PROJECT_NAME} PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
    target_link_options(${PROJECT_NAME} PUBLIC -fprofile-arcs -ftest-coverage)
endif ()
