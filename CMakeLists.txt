cmake_minimum_required(VERSION 3.15)

project(
        CloysterHPC
        DESCRIPTION "Cloyster HPC is a turnkey HPC cluster solution with an user-friendly installer"
        HOMEPAGE_URL "https://github.com/viniciusferrao/cloysterhpc"
        LANGUAGES CXX
)

option(DUMMY "Create a dummy executable" ON)

function(configureProject)
    if (DUMMY)
        add_definitions(-D_DUMMY_)
    endif()
endfunction()

configureProject()

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_executable(main ${SOURCE_FILES})

target_compile_features(main PRIVATE cxx_std_20)

find_library(newt
        NAMES newt newt-devel
        PATHS "/usr/local/Cellar/newt/0.52.21/include"
        REQUIRED)

find_package(Boost REQUIRED)

# -l flags for linking prog target
target_link_libraries(main
    newt
    Boost::headers
    )

# Add this compile definition for debug builds, this same logic works for
# target_compile_options, target_link_options, etc.
target_compile_definitions(main PRIVATE
    $<$<CONFIG:Debug>:
        _DEBUG_=1
    >
    )
