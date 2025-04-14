# This file set the required libraries for linking the project

set(COMMON_LIBS
        ${NEWT_LIBRARY}
        ${GLIBMM_LIBRARIES}
        ${STDC++FS}
        Boost::headers
        Boost::system
        Boost::thread
        CLI11::CLI11
        doctest::doctest
        fmt::fmt
        gsl::gsl-lite
        magic_enum::magic_enum
        resolv
        SDBusCpp::sdbus-c++
        spdlog::spdlog
)
