#!/bin/bash
# Cloyster Vagrant setup
# Created by Lucas Gracioso <contact@lbgracioso.net>
# SPDX-License-Identifier: Apache-2.0

# Check if the required arguments are provided
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <local_file_path> <vagrant_machine_name1> [<vagrant_machine_name2> ...]"
    exit 1
fi

local_file_path=$1
shift  # Shift the first argument (local_file_path) off the argument list

vagrant_machine_names=("$@")
vagrant_machine_names_string=$(IFS=' '; echo "${vagrant_machine_names[*]}")

# Check if the vagrant-scp plugin is installed
if ! vagrant plugin list | grep -q vagrant-scp; then
    echo "vagrant-scp plugin is not installed. Please install it using 'vagrant plugin install vagrant-scp'."
    exit 1
fi

# Run Vagrantfile

MACHINES="$vagrant_machine_names_string" vagrant up

# Loop through each Vagrant machine name, copy the local file, SSH, run Cloyster and then destroy the machine
for vagrant_machine_name in "${vagrant_machine_names[@]}"; do
    vagrant scp "$local_file_path" "$vagrant_machine_name":~/cloyster
    vagrant scp include/"$vagrant_machine_name".answerfile.ini "$vagrant_machine_name":~/answerfile.ini
    vagrant ssh "$vagrant_machine_name" -c "./cloyster -l 6 -a answerfile.ini"

    # Comment if you don't want the VM to be destroyed right after running Cloyster
    vagrant destroy "$vagrant_machine_name" -f
done