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

  dnf config-manager --set-enabled \
    "codeready-builder-for-rhel-${os_version}-x86_64-rpms"
  add_epel;
}

rocky() {
  if [ "$os_version" = "8" ]; then
    repo_name="powertools"
  elif [ "$os_version" = "9" ]; then
    repo_name="crb"
  fi

  dnf config-manager --set-enabled "$repo_name"
	dnf -y install epel-release
}

almalinux() {
  if [ "$os_version" = "8" ]; then
    repo_name="powertools"
  elif [ "$os_version" = "9" ]; then
    repo_name="crb"
  fi

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
dnf -y install rsync git gcc-c++ ninja-build cmake ccache cppcheck

if [ "$os_version" = "8" ]; then
  dnf -y install python3 python3-pip\* llvm-toolset compiler-rt \
    gcc-toolset-13 gcc-toolset-13-libubsan-devel gcc-toolset-13-libasan-devel
elif [ "$os_version" = "9" ]; then
  dnf -y install python pip libasan libubsan gcc-toolset-13 gcc-toolset-13-libubsan-devel gcc-toolset-13-libasan-devel llvm-toolset compiler-rt
fi

pip3 install --user conan

# Required libraries
dnf -y install newt-devel

echo
echo Development tools, packages and libraries were installed on your system.
echo
echo If compiling or running on EL8, please remember to source or activate the
echo environment file with the correct toolset compiler:
echo     \"source rhel-gcc-toolset-13.sh\"
echo
echo To proceed with the compilation please refer to the README.md file.
echo
