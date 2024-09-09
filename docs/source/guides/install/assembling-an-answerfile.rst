.. _assembling-an-answerfile:

========================
Assembling an answerfile
========================

Cloyster' CLI answerfile (used only in Command Line Interface) is a INI-based file that contains settings definitions and values used during Cloyster execution. In the following example answerfile, you specify various options (some options are obligatory and some optional). These options include the cluster information, time values, networking (external, management and application networks}, desired system iso file and the nodes to be used within the cluster.

Information
~~~~~~~~~~~

This section requires information about your cluster and your company.

.. code-block:: ini

    [information]
    cluster_name=cloyster
    company_name=cloyster-enterprises
    administrator_email=foo@example.com

Time
~~~~

This section requires the cluster timezone, timeserver and locale.

.. code-block:: ini

    [time]
    timezone=America/Sao_Paulo
    timeserver=0.br.pool.ntp.org
    locale=en_US.UTF-8  # Must follow the desired OS supported locales.

Hostname
~~~~~~~~

This section requires your machine hostname and domain name.

.. code-block:: ini

    [hostname]
    hostname=cloyster
    domain_name=cluster.example.com

Networks
~~~~~~~~

This section requires information about the cluster networks.

External
~~~~~~~~

This network is used by the management node to control the nodes out of band via the SP like BMC, FSP. If the BMCs are configured in shared mode, then this network can be combined with the management network.

.. code-block:: ini

    # Cloyster must have an external network
    [network_external]
    interface=enp1s0
    #ip_address=172.16.144.0
    #subnet_mask=255.255.255.0
    #network_address=172.16.144.1
    #gateway=192.168.122.1
    #domain_name=example.com
    #nameservers=146.164.36.7,146.164.36.15
    #mac_address=de:ad:be:ff:00:00

Management
~~~~~~~~~~

This network is used by the management node to install and manage the OS of the nodes. The MN and in-band NIC of the nodes are connected to this network. If you have a large cluster with service nodes, sometimes this network is segregated into separate VLANs for each service node.

.. code-block:: ini

    # Cloyster must have an management network
    [network_management]
    interface=enp8s0
    ip_address=172.26.255.254
    subnet_mask=255.255.0.0
    network_address=172.26.0.0
    #gateway=172.26.0.1
    domain_name=cluster.example.com
    #nameservers=172.26.0.1
    #mac_address=de:ad:be:ff:00:01

Application
~~~~~~~~~~~

This network is used by the applications on the compute nodes. Usually an IB network for HPC cluster.

.. code-block:: ini

    # Use the network_application if using a Infiniband
    # Must inform all options if enabled
    #[network_application]
    #interface=ib0
    #ip_address=172.26.0.0
    #subnet_mask=255.255.0.0
    #network_address=172.26.0.1
    #gateway=0.0.0.0
    #domain_name=cloysterhpc.example
    #nameservers=0.0.0.0,0.0.0.0
    #mac_address=de:ad:be:ff:00:01

Service
~~~~~~~~~~~

This network is used for internal services that support cluster operation but are separate from application traffic.

.. code-block:: ini

    # Double-commented options are optional
    #[network_service]
    #interface=enp2s0
    #ip_address=172.26.255.256
    #subnet_mask=255.255.0.0
    ##gateway=172.26.0.1
    #domain_name=cluster.example.com
    ##nameservers=172.26.0.1
    ##mac_address=de:ad:be:ff:00:01

System
~~~~~~

This section requires information about the path to an iso disk image with your desired OS.

.. code-block:: ini

    [system]
    # Full path to the disk image
    disk_image=/root/OracleLinux-R8-U7-x86_64-dvd.iso
    # Supported distros: rhel, ol, rocky, almalinux
    distro=ol
    version=8.7
    kernel=5.4.17-2136.302.6.1.el8uek.x86_64

Nodes
~~~~~

This section requires information about your nodes. If you need to add more than one MAC address, use a comma to separate.

.. code-block:: ini

    [node]
    prefix=n
    padding=2
    node_ip=172.26.0.1
    node_root_password=pwd
    sockets=1
    cores_per_socket=1
    threads_per_core=1
    bmc_username=admin
    bmc_password=admin
    bmc_serialport=0
    bmc_serialspeed=9600

Example of an answerfile
~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../../files/answerfile.ini.example
   :language: ini