cmake_minimum_required(VERSION 3.24)

project(CmakeConfigPackageTests LANGUAGES CXX)

# ---- Test as standalone project the exported config package ----

if(PROJECT_IS_TOP_LEVEL OR TEST_INSTALLED_VERSION)
  enable_testing()

  find_package(cloysterhpc CONFIG REQUIRED) # for intro, project_options, ...

  if(NOT TARGET cloysterhpc_options)
    message(FATAL_ERROR "Required config package not found!")
    return() # be strictly paranoid for Template Janitor github action! CK
  endif()
endif()

# ---- Dependencies ----

# Provide a simple smoke test to make sure that the CLI works and can display a --help message
add_test(NAME cli.has_help COMMAND main --help)

# Provide a test to verify that the version being reported from the application
# matches the version given to CMake. This will be important once you package
# your program. Real world shows that this is the kind of simple mistake that is easy
# to make, but also easy to test for.
add_test(NAME cli.version_matches COMMAND main --version)
set_tests_properties(cli.version_matches PROPERTIES PASS_REGULAR_EXPRESSION "${PROJECT_VERSION}")

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
message(STATUS "Test files found are: ${TEST_SOURCES}")

add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE cloysterhpc::cloysterhpc_warnings
          cloysterhpc::cloysterhpc_options
          doctest::doctest
          fmt::fmt
          spdlog::spdlog
          magic_enum::magic_enum
          gsl::gsl-lite
          boost::boost)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD})

if(WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(
    TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
    COMMAND_EXPAND_LISTS)
endif()

## enable compiler warnings
#if(NOT TEST_INSTALLED_VERSION)
#  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
#    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wpedantic -Wextra -Werror)
#  elseif(MSVC)
#    target_compile_options(${PROJECT_NAME} PUBLIC /W4 /WX)
#    target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
#  endif()
#endif()

# ---- Add CloysterHPCTests ----

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME ${PROJECT_NAME} COMMAND
# ${PROJECT_NAME})

if (cloysterhpc_ENABLE_CONAN)
  string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CMAKE_SYSTEM_PROCESSOR_LOWER)

  include(${doctest_DIR}/doctest-${CMAKE_BUILD_TYPE_LOWER}-${CMAKE_SYSTEM_PROCESSOR_LOWER}-data.cmake)
  include(${doctest_BUILD_DIRS_${CMAKE_BUILD_TYPE_UPPER}}/doctest.cmake)
else()
  include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
endif()

doctest_discover_tests(${PROJECT_NAME})

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(${PROJECT_NAME} PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(${PROJECT_NAME} PUBLIC -fprofile-arcs -ftest-coverage)
endif()

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
#catch_discover_tests(
#  tests
#  TEST_PREFIX
#  "unittests."
#  REPORTER
#  XML
#  OUTPUT_DIR
#        ../test-catch
#  OUTPUT_PREFIX
#  "unittests."
#  OUTPUT_SUFFIX
#  .xml)

# Add a file containing a set of constexpr tests
#add_executable(constexpr_tests ../test-catch/constexpr_tests.cpp)
#target_link_libraries(
#  constexpr_tests
#  PRIVATE cloysterhpc::cloysterhpc_warnings
#          cloysterhpc::cloysterhpc_options
#          doctest::doctest)
#
#catch_discover_tests(
#  constexpr_tests
#  TEST_PREFIX
#  "constexpr."
#  REPORTER
#  XML
#  OUTPUT_DIR
#        ../test-catch
#  OUTPUT_PREFIX
#  "constexpr."
#  OUTPUT_SUFFIX
#  .xml)

# Disable the constexpr portion of the test, and build again this allows us to have an executable that we can debug when
# things go wrong with the constexpr testing
#add_executable(relaxed_constexpr_tests ../test-catch/constexpr_tests.cpp)
#target_link_libraries(
#  relaxed_constexpr_tests
#  PRIVATE cloysterhpc::cloysterhpc_warnings
#          cloysterhpc::cloysterhpc_options
#          doctest::doctest)
#
#target_compile_definitions(relaxed_constexpr_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)
#
#catch_discover_tests(
#  relaxed_constexpr_tests
#  TEST_PREFIX
#  "relaxed_constexpr."
#  REPORTER
#  XML
#  OUTPUT_DIR
#        ../test-catch
#  OUTPUT_PREFIX
#  "relaxed_constexpr."
#  OUTPUT_SUFFIX
#  .xml)
