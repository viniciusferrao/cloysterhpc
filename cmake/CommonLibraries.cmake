# This file set the required libraries for linking the project

set(COMMON_LIBS
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
  cryptopp::cryptopp
  SDBusCpp::sdbus-c++
  lfreist-hwinfo::hwinfo)
