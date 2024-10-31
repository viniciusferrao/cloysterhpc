#!/bin/sh
#
# This file should be executed to enable the GCC 13 toolset on RHEL.
# It is meant to be sourced by the user's shell before any compilation,
# including Conan builds. If running from an IDE please remember to check this
# file as the environment.

source scl_source enable gcc-toolset-13

sudo dnf install -y gcc-toolset-13-libasan-devel gcc-toolset-13-libubsan-devel
sudo dnf install -y perl perl-File-Copy perl-Compare perl-Thread perl-Thread-Queue perl-open perl-FindBin
