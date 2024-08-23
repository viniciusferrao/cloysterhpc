# Ansible role for Cloyster
An Ansible role that provision a Vagrant virtual machine with Cloyster and runs it, for tests purpose.

# Requirements
- Ansible
- Vagrant
- Cloyster binary
- A ISO for the nodes (WIP)

# Example Playbook
```yaml
---
- name: Cloyster Vagrant Setup
  hosts: localhost
  become: yes
  gather_facts: false
  roles:
    - cloyster_setup
```

# Example Vagrantfile
**This Vagrantfile should be in the same directory as your playbook**

```
 # -*- mode: ruby -*-
# vi: set ft=ruby :

require 'yaml'

# Load the shared config file
config_file = File.join(File.dirname(__FILE__), 'roles/cloyster_setup/vars/machines.yml')
config = YAML.load_file(config_file)
machines = config['machines']

specified_machines = ENV['MACHINES'].to_s.split(',')

Vagrant.configure("2") do |config|
  config.vm.box_check_update = false

  machines.each do |name, conf|
    next unless specified_machines.empty? || specified_machines.include?(name)

    config.vm.define "#{name}" do |m|
      m.vm.box = "#{conf['box']}"
      m.vm.network 'private_network', ip: "#{conf['vagrant_external_network_address']}", name: "external_network"
      m.vm.network 'private_network', ip: "#{conf['vagrant_management_network_address']}", name: "management_network"
      m.vm.hostname = "cloyster.hpc"
      m.vm.provider 'libvirt' do |lv|
        lv.memory = conf['memory']
        lv.cpus = conf['cpus']
        lv.host = 'localhost'
        lv.driver = 'kvm'
        lv.uri = 'qemu:///system'
      end
      m.vm.provision "ansible" do |ansible|
        ansible.playbook = "roles/cloyster_setup/provision/#{conf['provision']}"
        ansible.compatibility_mode = "2.0"
      end
      m.vm.synced_folder '.', '/vagrant', type: "rsync"
    end
  end
end
```

# How to use
Run `ansible-playbook setup.yml --extra-vars "cloyster_binary_path=/path/to/local/cloyster/binary vagrant_machine_name=machine_name iso_image_path=/path/to/iso/image"` in the same folder of "setup.yml" or the playbook you created.

If you don't want to clean up (remove the virtual machine) after Cloyster ends, set `"cleanup_needed=false"` on the `--extra-vars`.