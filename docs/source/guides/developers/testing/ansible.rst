.. _ansible:

Ansible
=======

Configuration
-------------

Please run all commands in this file as root or with elevated privileges.

Dependencies
~~~~~~~~~~~~

* Ansible
* Vagrant
* Cloyster binary
* A ISO for the nodes (WIP)

Example Playbook
~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../../../files/ansible/setup.yml.example
   :language: yaml

Example Vagrantfile
~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../../../files/ansible/Vagrantfile.example
   :language: ruby

Running
-------

.. note::

   This should be used inside Ansible (/ansible) directory

Run ``ansible-playbook setup.yml --extra-vars "cloyster_binary_path=/path/to/local/cloyster/binary vagrant_machine_name=machine_name iso_image_path=/path/to/iso/image"`` in the same folder of `"setup.yml"` or the playbook you created.
If you don't want to clean up (remove the virtual machine) after Cloyster ends, set ``"cleanup_needed=false"`` on the --extra-vars.

For the current list of operating systems supported, read section ":doc:`System Requirements <../../../overview/sys_os_requirements>`"