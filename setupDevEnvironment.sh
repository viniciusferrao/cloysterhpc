#!/bin/sh
#
# Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
# SPDX-License-Identifier: Apache-2.0
#
# This file is part of CloysterHPC
# https://github.com/viniciusferrao/cloysterhpc
#
# Script to ease development environment setup on EL8 systems

# Stop execution in case of any error (add x for debugging)
set -e

# OS relevant settings
redhat() {
	subscription-manager refresh
	dnf config-manager --set-enabled codeready-builder-for-rhel-8-x86_64-rpms
	dnf -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
}

rocky() {
	dnf config-manager --set-enabled powertools
	dnf -y install epel-release
}

almalinux() {
	dnf config-manager --set-enabled powertools
	dnf -y install epel-release
}

oracle() {
	dnf config-manager --set-enabled ol8_codeready_builder
	dnf -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
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
		exit 1
		;;
esac

# Build toolset and packages
dnf -y install git cmake ccache ninja-build llvm-toolset gcc-toolset-12\*
pip3 install conan

# Required libraries
dnf -y install newt-devel

echo
echo Development tools, packages and libraries were installed on your system.
echo To proceed with the compilation please refer to the README.md file.
echo
