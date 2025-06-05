#!/bin/sh
#
# Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
# SPDX-License-Identifier: Apache-2.0
#
# This file is part of CloysterHPC
# https://github.com/viniciusferrao/cloysterhpc
#
# Script to ease development environment setup on Enterprise Linux (EL) systems

# Stop execution in case of any error (add x for debugging)
set -e

# Check if it's not running as root
if [ "$(id -u)" -ne 0 ]; then
  # Check if sudo is available
  if command -v sudo >/dev/null; then
    # Override dnf to use sudo
    dnf() {
      sudo /usr/bin/dnf "$@"
    }
  else
    echo \
      "sudo is required but not installed. Please install sudo or run as root."
    exit 1
  fi
fi

# Grab EL version from RPM
os_version=$(rpm -E %rhel)

# Helper functions
add_epel() {
  dnf -y install \
    "https://dl.fedoraproject.org/pub/epel/epel-release-latest-${os_version}.noarch.rpm"
}

# OS relevant settings
redhat() {
  if [ "$(id -u)" -eq 0 ]; then
    subscription-manager refresh
  else
    sudo subscription-manager refresh
  fi

  if [ "$os_version" -eq 10 ]; then
    dnf config-manager --set-enabled \
      "codeready-builder-beta-for-rhel-${os_version}-x86_64-rpms"
  else
    dnf config-manager --set-enabled \
      "codeready-builder-for-rhel-${os_version}-x86_64-rpms"
  fi

  add_epel;
}

rocky() {
  case "$os_version" in
    8)
      repo_name="powertools"
      ;;
    9|10)
      repo_name="crb"
      ;;
  esac

  dnf config-manager --set-enabled "$repo_name"
  dnf -y install epel-release
}

almalinux() {
  case "$os_version" in
    8)
      repo_name="powertools"
      ;;
    9|10)
      repo_name="crb"
      ;;
  esac

  dnf config-manager --set-enabled "$repo_name"
  dnf -y install epel-release
}

oracle() {
  dnf config-manager --set-enabled "ol${os_version}_codeready_builder"
  add_epel;
}

#
# Entrypoint
#
echo Setting up development environment for CloysterHPC
echo

case $(cut -f 3 -d : /etc/system-release-cpe) in
	redhat)
		redhat;
		;;
	rocky)
		rocky;
		;;
	almalinux)
		almalinux;
		;;
	oracle)
		oracle;
		;;
	*)
		echo Unable to properly identify the running OS. Aborting.
		exit 2
		;;
esac

# Build toolset, packages and utils
dnf -y install rsync git gcc-c++ gdb cmake ccache ninja-build llvm-toolset \
  lldb compiler-rt

case "$os_version" in
  8)
    dnf -y install python3 python3-pip\* gcc-toolset-14 \
      gcc-toolset-14-libubsan-devel gcc-toolset-14-libasan-devel cppcheck \
      perl-File-Copy perl-File-Compare perl-Thread-Queue
    ;;
  9)
    dnf -y install python pip libasan libubsan gcc-toolset-14 \
      gcc-toolset-14-libubsan-devel gcc-toolset-14-libasan-devel cppcheck \
      glibmm24 glibmm24-devel \
      perl-File-Copy perl-File-Compare perl-Thread-Queue perl-FindBin

    ;;
  10)
    dnf -y install python pip libubsan libasan liblsan libtsan libhwasan \
      glibmm-2.68 glibmm-2.68-devel \
      perl-File-Copy perl-File-Compare perl-Thread-Queue
    ;;
esac

# Install Conan as user
pip3 install --user conan

# Required libraries
dnf -y install newt-devel

echo
echo Development tools, packages and libraries were installed on your system.
echo
echo If compiling or running on EL8 or EL9, please remember to source or
echo activate the environment file with the correct toolset compiler:
echo     \"source rhel-gcc-toolset-14.sh\"
echo
echo To proceed with the compilation please refer to the README.md file.
echo
