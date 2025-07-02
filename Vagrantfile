# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vagrant.plugins = "vagrant-libvirt"

  config.vm.define "headnode" do |config|
    config.vm.box = "rockylinux/9"
    config.vm.box_version = "4.0.0"
  end
end

