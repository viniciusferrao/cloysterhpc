cmake_minimum_required(VERSION 3.16...3.25)

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

# Library: doctest
find_package(doctest CONFIG REQUIRED)

project(CmakeConfigPackageTests LANGUAGES CXX)

# List all files containing tests. (Change as needed)
set(TESTFILES        # All .cpp files in tests/
        src/main.cpp
        src/network.cpp
        )

set(TEST_MAIN unit_test)   # Default name for test executable (change if you wish).
set(TEST_RUNNER_PARAMS "")  # Any arguments to feed the test runner (change as needed).

# --------------------------------------------------------------------------------
#                         Make Tests (no change needed).
# --------------------------------------------------------------------------------
add_executable(${TEST_MAIN} ${TESTFILES})
target_link_libraries(${TEST_MAIN}
        PUBLIC project_options project_warnings
        PRIVATE
			doctest::doctest
			fmt::fmt
			CLI11::CLI11
			Boost::headers
			Boost::system
			Boost::thread
			spdlog::spdlog
			gsl::gsl-lite
			magic_enum::magic_enum
			)

set_target_properties(${TEST_MAIN} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

set_target_properties(${TEST_MAIN} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

add_test(
        # Use some per-module/project prefix so that it is easier to run only tests for this module
        NAME ${LIBRARY_NAME}.${TEST_MAIN}
        COMMAND ${TEST_MAIN} ${TEST_RUNNER_PARAMS})
