#!/bin/sh
# Cloyster Vagrant setup
# Created by Lucas Gracioso <contact@lbgracioso.net>
# SPDX-License-Identifier: Apache-2.0

# Check if the required arguments are provided
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 <local_file_path> <vagrant_machine_name> <iso_image_path>"
    exit 1
fi

local_file_path=$1
vagrant_machine_name=$2
iso_image_path=$3

# Check if vagrant-scp plugin is installed
if ! vagrant plugin list | grep -q vagrant-scp; then
    echo "vagrant-scp plugin is not installed. Please install it using 'vagrant plugin install vagrant-scp'."
    exit 1
fi

configure_disk() {
    machine_name="$1"
    new_filesystem="vdb"
    disk_image="/var/lib/libvirt/images/${machine_name}_extra.img"

    # Create disk
    qemu-img create -f raw "$disk_image" 50G

    # Attach and configure
    virsh attach-disk "vagrant_$machine_name" "$disk_image" "$new_filesystem" --cache none --subdriver raw --persistent

    # Disk setup
    vagrant ssh "$machine_name" -c "sudo parted /dev/$new_filesystem mklabel msdos"
    vagrant ssh "$machine_name" -c "sudo parted /dev/$new_filesystem mkpart primary xfs 2048s 100%"
    vagrant ssh "$machine_name" -c "sudo partprobe /dev/$new_filesystem"
    vagrant ssh "$machine_name" -c "sudo mkfs.xfs /dev/${new_filesystem}1"

    # Mount and update /etc/fstab
    vagrant ssh "$machine_name" -c "sudo mkdir /opt"
    vagrant ssh "$machine_name" -c "sudo mount /dev/${new_filesystem}1 /opt"
    vagrant ssh "$machine_name" -c "echo \"/dev/vdb1   /opt    xfs    defaults    0 0\" | sudo tee -a /etc/fstab"

    # Create /opt/iso path
    vagrant ssh "$machine_name" -c "sudo mkdir /opt/iso"

    vagrant ssh "$machine_name" -c "sudo chown vagrant:vagrant /opt/ -R"

    # Reload machine
    vagrant reload "$vagrant_machine_name"
}

deploy_cloyster() {
    machine_name="$1"
    local_file_path="$2"
    iso_image_path="$3"

    # Copy files to VM
    vagrant scp "$local_file_path" "$machine_name":~/cloyster
    vagrant scp "$iso_image_path" "$machine_name":/opt/iso
    vagrant scp include/"$machine_name".answerfile.ini "$machine_name":~/answerfile.ini

    # Run Cloyster
    vagrant ssh "$machine_name" -c "sudo ./cloyster -l 6 -a answerfile.ini -u"
}

destroy_vm_and_cleanup() {
    machine_name="$1"

    vagrant destroy "$machine_name" -f
    rm -f "/var/lib/libvirt/images/${machine_name}_extra.img"
}

# Run Vagrantfile
MACHINES="$vagrant_machine_name" vagrant up

# Set up for the specified Vagrant machine
configure_disk "$vagrant_machine_name"
deploy_cloyster "$vagrant_machine_name" "$local_file_path" "$iso_image_path"
destroy_vm_and_cleanup "$vagrant_machine_name"
