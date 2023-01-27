#!/bin/sh
#
# This file should be executed to enable the GCC 12 toolset on RHEL 8.
# It is meant to be sourced by the user's shell before any compilation,
# including Conan builds. If running from an IDE please remeber to check this
# file as the environment.

source scl_source enable gcc-toolset-12
