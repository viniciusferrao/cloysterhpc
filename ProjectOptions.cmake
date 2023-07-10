include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)

macro(cloysterhpc_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(cloysterhpc_setup_options)
  option(cloysterhpc_ENABLE_HARDENING "Enable hardening" ON)
  option(cloysterhpc_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    cloysterhpc_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    cloysterhpc_ENABLE_HARDENING
    OFF)

  cloysterhpc_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR cloysterhpc_PACKAGING_MAINTAINER_MODE)
    option(cloysterhpc_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(cloysterhpc_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(cloysterhpc_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(cloysterhpc_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(cloysterhpc_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(cloysterhpc_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(cloysterhpc_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(cloysterhpc_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(cloysterhpc_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(cloysterhpc_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(cloysterhpc_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(cloysterhpc_ENABLE_PCH "Enable precompiled headers" OFF)
    option(cloysterhpc_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(cloysterhpc_ENABLE_IPO "Enable IPO/LTO" ON)
    option(cloysterhpc_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(cloysterhpc_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(cloysterhpc_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(cloysterhpc_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(cloysterhpc_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(cloysterhpc_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(cloysterhpc_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(cloysterhpc_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(cloysterhpc_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(cloysterhpc_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(cloysterhpc_ENABLE_PCH "Enable precompiled headers" OFF)
    option(cloysterhpc_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      cloysterhpc_ENABLE_IPO
      cloysterhpc_WARNINGS_AS_ERRORS
      cloysterhpc_ENABLE_USER_LINKER
      cloysterhpc_ENABLE_SANITIZER_ADDRESS
      cloysterhpc_ENABLE_SANITIZER_LEAK
      cloysterhpc_ENABLE_SANITIZER_UNDEFINED
      cloysterhpc_ENABLE_SANITIZER_THREAD
      cloysterhpc_ENABLE_SANITIZER_MEMORY
      cloysterhpc_ENABLE_UNITY_BUILD
      cloysterhpc_ENABLE_CLANG_TIDY
      cloysterhpc_ENABLE_CPPCHECK
      cloysterhpc_ENABLE_COVERAGE
      cloysterhpc_ENABLE_PCH
      cloysterhpc_ENABLE_CACHE)
  endif()

  cloysterhpc_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (cloysterhpc_ENABLE_SANITIZER_ADDRESS OR cloysterhpc_ENABLE_SANITIZER_THREAD OR cloysterhpc_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(cloysterhpc_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(cloysterhpc_global_options)
  if(cloysterhpc_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    cloysterhpc_enable_ipo()
  endif()

  cloysterhpc_supports_sanitizers()

  if(cloysterhpc_ENABLE_HARDENING AND cloysterhpc_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR cloysterhpc_ENABLE_SANITIZER_UNDEFINED
       OR cloysterhpc_ENABLE_SANITIZER_ADDRESS
       OR cloysterhpc_ENABLE_SANITIZER_THREAD
       OR cloysterhpc_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${cloysterhpc_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${cloysterhpc_ENABLE_SANITIZER_UNDEFINED}")
    cloysterhpc_enable_hardening(cloysterhpc_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(cloysterhpc_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(cloysterhpc_warnings INTERFACE)
  add_library(cloysterhpc_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  cloysterhpc_set_project_warnings(
    cloysterhpc_warnings
    ${cloysterhpc_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(cloysterhpc_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(cloysterhpc_options)
  endif()

  include(cmake/Sanitizers.cmake)
  cloysterhpc_enable_sanitizers(
    cloysterhpc_options
    ${cloysterhpc_ENABLE_SANITIZER_ADDRESS}
    ${cloysterhpc_ENABLE_SANITIZER_LEAK}
    ${cloysterhpc_ENABLE_SANITIZER_UNDEFINED}
    ${cloysterhpc_ENABLE_SANITIZER_THREAD}
    ${cloysterhpc_ENABLE_SANITIZER_MEMORY})

  set_target_properties(cloysterhpc_options PROPERTIES UNITY_BUILD ${cloysterhpc_ENABLE_UNITY_BUILD})

  if(cloysterhpc_ENABLE_PCH)
    target_precompile_headers(
      cloysterhpc_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(cloysterhpc_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    cloysterhpc_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(cloysterhpc_ENABLE_CLANG_TIDY)
    cloysterhpc_enable_clang_tidy(cloysterhpc_options ${cloysterhpc_WARNINGS_AS_ERRORS})
  endif()

  if(cloysterhpc_ENABLE_CPPCHECK)
    cloysterhpc_enable_cppcheck(${cloysterhpc_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(cloysterhpc_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    cloysterhpc_enable_coverage(cloysterhpc_options)
  endif()

  if(cloysterhpc_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(cloysterhpc_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(cloysterhpc_ENABLE_HARDENING AND NOT cloysterhpc_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR cloysterhpc_ENABLE_SANITIZER_UNDEFINED
       OR cloysterhpc_ENABLE_SANITIZER_ADDRESS
       OR cloysterhpc_ENABLE_SANITIZER_THREAD
       OR cloysterhpc_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    cloysterhpc_enable_hardening(cloysterhpc_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
