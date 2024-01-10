#!/bin/sh
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

## Loop through each Vagrant machine name, create a new disk and attach it to the VM
for vagrant_machine_name in "${vagrant_machine_names[@]}"; do

    # Create disk
    qemu-img create -f raw /var/lib/libvirt/images/"$vagrant_machine_name"_extra.img 50G # Create a new disk image

    # Attach and configure
    new_filesystem="vdb"
    virsh attach-disk vagrant_"$vagrant_machine_name" /var/lib/libvirt/images/"$vagrant_machine_name"_extra.img $new_filesystem --cache none --subdriver raw --persistent # Attach the new disk

    vagrant ssh "$vagrant_machine_name" -c "sudo parted /dev/$new_filesystem mklabel msdos" # Create a new partition table
    vagrant ssh "$vagrant_machine_name" -c "sudo parted /dev/$new_filesystem mkpart primary xfs 2048s 100%" # Create a new primary partition on the specified disk
    vagrant ssh "$vagrant_machine_name" -c "sudo partprobe /dev/$new_filesystem" # Inform the kernel about the changes
    vagrant ssh "$vagrant_machine_name" -c "sudo mkfs.xfs /dev/$new_filesystem\1" # Call mkfs.xfs to make the new filesystem
    vagrant ssh "$vagrant_machine_name" -c "sudo mkdir /opt" # Make /opt dir if it doesn't exist
    vagrant ssh "$vagrant_machine_name" -c "sudo mount /dev/$new_filesystem\1 /opt" # Mount the new filesystem to /opt
    vagrant ssh "$vagrant_machine_name" -c "echo \"/dev/vdb1   /opt    xfs    defaults    0 0\" | sudo tee -a /etc/fstab" # Add to /etc/fstab

    # Reload
    vagrant reload "$vagrant_machine_name"
done

# Loop through each Vagrant machine name, copy the local file, SSH, run Cloyster and then destroy the machine
for vagrant_machine_name in "${vagrant_machine_names[@]}"; do
    vagrant scp "$local_file_path" "$vagrant_machine_name":~/cloyster
    vagrant scp include/"$vagrant_machine_name".answerfile.ini "$vagrant_machine_name":~/answerfile.ini
    vagrant ssh "$vagrant_machine_name" -c "./cloyster -l 6 -a answerfile.ini"

    # Comment if you don't want the VM or the extra disk to be destroyed right after running Cloyster
    vagrant destroy "$vagrant_machine_name" -f
    rm -f /var/lib/libvirt/images/"$vagrant_machine_name"_extra.img

done