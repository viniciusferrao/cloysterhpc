# Add all source files except main.cpp
file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB PRESENTER_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/presenter/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/view/*.cpp)
list(REMOVE_ITEM SOURCE_FILES ${PRESENTER_SOURCE_FILES})
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

message(STATUS "Source files found are: ${SOURCE_FILES}")

include(../cmake/CommonLibraries.cmake)

# Build the presenter on its onw object, to be faster
# Not wo
add_library(cloysterhpc_presenter_object OBJECT ${PRESENTER_SOURCE_FILES})
target_include_directories(cloysterhpc_presenter_object PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_link_system_libraries(
        cloysterhpc_presenter_object
        PRIVATE
        SimpleIni::SimpleIni
        SDBusCpp::sdbus-c++
        magic_enum::magic_enum
        gsl::gsl-lite
        fmt::fmt
        spdlog::spdlog
        glibmm::glibmm
        Boost::headers
        ${NEWT_LIBRARY}
        )

# Build a library and a ${cloysterhpc_BINARY_NAME} binary that links to the library
add_library(cloysterhpc_object OBJECT ${SOURCE_FILES})
# add_library(cloysterhpc_static STATIC $<TARGET_OBJECTS:cloysterhpc_object>)
add_library(cloysterhpc_static STATIC $<TARGET_OBJECTS:cloysterhpc_object> $<TARGET_OBJECTS:cloysterhpc_presenter_object>)
add_executable(${cloysterhpc_BINARY_NAME} main.cpp)

target_link_libraries(
        ${cloysterhpc_BINARY_NAME}
        PRIVATE
        cloysterhpc_static
        cloysterhpc::cloysterhpc_options
        cloysterhpc::cloysterhpc_warnings)

target_link_system_libraries(
        ${cloysterhpc_BINARY_NAME}
        PRIVATE
        ${COMMON_LIBS})

target_link_libraries(
        cloysterhpc_object
        PRIVATE
        cloysterhpc::cloysterhpc_options
        cloysterhpc::cloysterhpc_warnings)

target_link_system_libraries(
        cloysterhpc_object
        PRIVATE
        ${COMMON_LIBS})

target_include_directories(cloysterhpc_static PRIVATE
        "${CMAKE_BINARY_DIR}/configured_files/include")
target_include_directories(cloysterhpc_static ${WARNING_GUARD} PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_include_directories(cloysterhpc_object PRIVATE
        "${CMAKE_BINARY_DIR}/configured_files/include")
target_include_directories(cloysterhpc_object ${WARNING_GUARD} PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_include_directories(${cloysterhpc_BINARY_NAME} PRIVATE
        "${CMAKE_BINARY_DIR}/configured_files/include")
target_include_directories(${cloysterhpc_BINARY_NAME} ${WARNING_GUARD} PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

# Set target locations to the root folder of the project in /bin and /lib.
# WARNING: This will break multiple target compile, disable it if necessary
# This is only for convenience
#set_target_properties(${cloysterhpc_BINARY_NAME}
#    PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
#    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
#    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
#)
