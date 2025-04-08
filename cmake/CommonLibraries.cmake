# This file set the required libraries for linking the project

set(COMMON_LIBS
        ${NEWT_LIBRARY}
        fmt::fmt
        Boost::headers
        Boost::system
        Boost::thread
        Boost::program_options
        spdlog::spdlog
        gsl::gsl-lite
        ${GLIBMM_LIBRARIES}
        magic_enum::magic_enum
        resolv
        ${STDC++FS}
        doctest::doctest
        SDBusCpp::sdbus-c++)
