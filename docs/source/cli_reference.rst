=====================================
CloysterHPC Operations Guide
=====================================

CloysterHPC is a turnkey automation engine for rapid HPC cluster rollouts. It
aligns firmware, operating system, provisioning and middleware components so you
can light up a production-ready environment in record time.

Features
--------
- Unified provisioning for OpenHPC 3 software stacks with ready-to-run SLURM queuing.
- Choice of xCAT or Confluent bare-metal onboarding, using the same answerfile workflow.
- Automated network, storage and management fabric setup, including InfiniBand support and NFS exports.
- Built-in hardening through optional roles for auditd, AIDE, fail2ban, SELinux tuning, firewalld, and SSH lockdown.
- Integrated package orchestration with Spack, custom OpenHPC components, and mirror management.
- Repeatable workflows with smart skip/force controls for every step of the installer pipeline.

.. _answerfile-reference:

Answerfile Reference
--------------------

CloysterHPC relies on an INI-style *answerfile* to capture every decision about
the cluster. The installer consults the sections below in sequence. Required
keys must be present whenever their section is included; empty values are
treated as not provided.

.. note::
   Optional network sections (``network_service`` and ``network_application``)
   are evaluated only when present. If you add them, populate all required
   fields just like the mandatory networks.

[network_management]
~~~~~~~~~~~~~~~~~~~~
- **interface** *(required)* – Adapter dedicated to management traffic.
- **ip_address** *(required)* – Head node management address; drives IP/CIDR
  calculations and NetworkManager provisioning.
- **mac_address** *(optional)* – Persisted in the connection profile when
  provided.
- **subnet_mask** *(required)* – Defines CIDR width for the management subnet.
- **domain_name** *(required)* – Combined with the hostname for the cluster
  FQDN and hosts file entries.
- **gateway** *(optional)* – Default gateway for management traffic.
- **nameservers** *(optional, comma-separated)* – Overrides DNS resolvers for
  the management network.

[network_external]
~~~~~~~~~~~~~~~~~~
- **interface** *(required)* – Outbound interface for package mirrors.
- **ip_address** *(optional)* – Static address; if omitted, the installer reads
  the live interface configuration.
- **mac_address** *(optional)* – Binds the connection profile to a specific NIC.
- **subnet_mask**, **gateway**, **domain_name**, **nameservers** – Same
  semantics as the management network. Missing values trigger smart discovery
  via NetworkManager.

[network_service]
~~~~~~~~~~~~~~~~~
Only parsed when present.

- **interface**, **ip_address**, **mac_address**, **subnet_mask**, **gateway**,
  **domain_name**, **nameservers** – Adds a dedicated out-of-band service
  network, informing chrony access lists and firewalld zones.

[network_application]
~~~~~~~~~~~~~~~~~~~~~
Optional high-performance fabric (InfiniBand/Ethernet storage).

- **interface**, **ip_address**, **mac_address**, **subnet_mask**, **gateway**,
  **domain_name**, **nameservers** – Provides addressing for the fabric, feeding
  OFED setup, firewall rules, and host inventories.

[information]
~~~~~~~~~~~~~
- **cluster_name** *(required)* – Names the cluster and seeds the SLURM
  ``ClusterName``.
- **company_name** *(required)* – Available for reports and templated branding.
- **administrator_email** *(required)* – Target for operational notifications.

[time]
~~~~~~
- **timezone** *(required)* – Applied via ``timedatectl``.
- **timeserver** *(required)* – Upstream source for chrony.
- **locale** *(required)* – Configured through ``localectl`` for consistent
  regional settings.

[hostname]
~~~~~~~~~~
- **hostname** *(required)* – Head node short name.
- **domain_name** *(required)* – Forms the FQDN and TLS subjects.

[system]
~~~~~~~~
- **disk_image** *(required)* – Path to the installation ISO or disk image.
- **distro** *(required)* – Supported values include ``rocky``, ``rhel`` and
  ``ol``.
- **version** *(required)* – OS release (e.g. ``9.6``).
- **kernel** *(optional)* – Records a desired kernel version for reference.
- **provisioner** *(required)* – Choose ``xcat`` or ``confluent``; this toggles
  the provisioning roles executed later.

[node]
~~~~~~
Template for compute nodes. If a node-specific section omits a field, the
installer copies it from here.

- **prefix** *(required)* – Base hostname prefix (``n`` results in ``n01``).
- **padding** *(required)* – Zero padding width applied to node numbers.
- **node_ip** *(required)* – Starting IP used for automatic addressing.
- **node_root_password** *(required)* – Root password deployed to each node.
- **sockets**, **cores_per_socket**, **threads_per_core**, **cpus_per_node**,
  **real_memory** *(required)* – Hardware topology mirrored into workload
  manager settings.
- **bmc_username**, **bmc_password**, **bmc_serialport**, **bmc_serialspeed**
  *(required)* – Credentials and console configuration for provisioning tools.

[node.N]
~~~~~~~~
One section per node (``node.1``, ``node.2`` …). Gaps in numbering are allowed.

- **hostname** *(optional)* – Override the templated hostname.
- **node_ip** *(optional)* – Override the IP sequence.
- **mac_address** *(required per node)* – Validated before the run; missing
  values stop the process.
- **node_root_password**, **sockets**, **cores_per_socket**,
  **threads_per_core**, **bmc_* fields** *(optional overrides)* – Fall back to
  the template when empty.

[postfix]
~~~~~~~~~
Optional mail relay integration.

- **destination** *(required when section exists)* – Comma-separated recipients.
- **profile** *(required)* – ``local``, ``relay`` or ``sasl``.
- **smtpd_tls_cert_file**, **smtpd_tls_key_file** *(required)* – TLS assets for
  the chosen profile.

``[postfix.relay]`` *(Relay profile only)*
    - **server**, **port** *(required)* – Upstream relay endpoint.

``[postfix.sasl]`` *(SASL profile only)*
    - **server**, **port**, **username**, **password** *(required)* – Authenticated
      SMTP credentials.

[ofed]
~~~~~~
Optional InfiniBand settings.

- **kind** *(optional)* – ``inbox`` or vendor distributions such as ``mellanox``.
- **version** *(optional)* – Specific vendor revision; defaults to ``latest``.

[slurm]
~~~~~~~
Mandatory for SLURM deployments (default).

- **mariadb_root_password** *(required)* – Secures the local database.
- **slurmdb_password** *(required)* – Credential for the SLURM DB user.
- **storage_password** *(required)* – Stored in ``slurmdbd.conf``.
- **partition_name** *(required)* – Creates the default queue.

Other Sections
~~~~~~~~~~~~~~
Hooks for toolchains (`loadTools`, `loadNVHPC`) exist for future expansion. The
installer ignores unknown groups, so you can safely add custom notes using
commented lines.

Answerfile Example
------------------

The sample below adapts the ``rocky9-confluent.ini`` blueprint for a two-node
cluster with Confluent provisioning and Mellanox OFED. Use it as a starting
point and adjust networking, credentials, and node counts to fit your
infrastructure.

.. code-block:: ini
   :caption: rocky9-confluent.ini (excerpt)

   [information]
   cluster_name=cloyster
   company_name=cloyster-enterprises
   administrator_email=foo@example.com

   [time]
   timezone=America/Sao_Paulo
   timeserver=0.br.pool.ntp.org
   locale=en_US.utf8

   [hostname]
   hostname=cloyster
   domain_name=cluster.example.com

   [network_external]
   interface=enp2s1
   domain_name=cluster.external.example.com

   [network_management]
   interface=enp2s2
   ip_address=192.168.30.254
   subnet_mask=255.255.255.0
   gateway=192.168.122.1
   domain_name=cluster.management.example.com
   nameservers=192.168.122.1

   [network_application]
   interface=ib0
   ip_address=172.26.0.0
   subnet_mask=255.255.0.0

   [network_service]
   interface=enp3s0
   ip_address=172.25.0.0
   subnet_mask=255.255.0.0

   [system]
   disk_image=/opt/iso/Rocky-9.6-x86_64-dvd.iso
   distro=rocky
   version=9.6
   provisioner=confluent

   [ofed]
   kind=mellanox
   version=latest-2.9-LTS

   [slurm]
   mariadb_root_password=xxxxxx
   slurmdb_password=xxxxxx
   storage_password=xxxxxx
   partition_name=batch

   [node]
   prefix=n
   padding=2
   node_ip=192.168.30.1
   node_root_password=pwd
   sockets=1
   cpus_per_node=1
   cores_per_socket=1
   threads_per_core=1
   real_memory=4096
   bmc_username=admin
   bmc_password=admin
   bmc_serialport=0
   bmc_serialspeed=9600

   [node.1]
   mac_address=ca:fe:de:ad:be:ef
   node_ip=192.168.30.1
   bmc_address=10.0.0.2

   [node.2]
   mac_address=ca:fe:de:ad:be:02
   node_ip=192.168.30.2
   bmc_address=10.0.0.3

Running CloysterHPC
-------------------

Launch the installer with elevated privileges. A typical unattended deployment
looks like this:

.. code-block:: bash

   cloysterhpc -a some-answerfile.ini -l6 -u

The ``-u`` switch bypasses the interactive confirmation, ``-l6`` amplifies log
verbosity, and ``-a`` injects your answerfile blueprint.

Command-Line Options
--------------------

Flags are processed with CLI11 and behave as follows:

.. list-table::
   :header-rows: 1

   * - Flag(s)
     - Purpose
     - Default
   * - ``-v`` / ``--version``
     - Print product version and exit
     - Off
   * - ``-r`` / ``--root``
     - Force root mode when running under sudo wrappers
     - Off
   * - ``-d`` / ``--dry``
     - Log actions without applying system changes
     - Off
   * - ``-c`` / ``--cli``
     - Reserved for future interactive workflows
     - Off
   * - ``-D`` / ``--daemon``
     - Placeholder for headless operation
     - Off
   * - ``--disable-mirrors``
     - Skip bundled mirror URLs and rely on upstream repositories
     - Mirrors enabled
   * - ``--mirror-url <url>``
     - Override the base mirror URL used for repository templating
     - ``https://mirror.versatushpc.com.br``
   * - ``--beegfs-version <tag>``
     - Select the BeeGFS version for client deployments
     - ``beegfs_7.3.3``
   * - ``--xcat-version <tag>``
     - Set the xCAT release when that provisioner is selected
     - ``latest``
   * - ``--zabbix-version <tag>``
     - Define the monitoring stack version
     - ``6.4``
   * - ``-l`` / ``--log-level <1-6>``
     - Adjust log verbosity (1=trace … 6=fatal)
     - ``3``
   * - ``-a`` / ``--answerfile <path>``
     - Load configuration from the specified answerfile
     - Not set
   * - ``--skip <step>``
     - Skip named execution steps (e.g. ``network``, ``check-kernel``,
       ``infiniband``, ``spack``, ``disk-checksum``)
     - None
   * - ``--force <step>``
     - Force specific steps even when already satisfied (e.g. ``backups``,
       ``infiniband-install``, ``genimage``, ``xcat-patch``)
     - None
   * - ``--stop-after <step>``
     - Halt after a named step for troubleshooting (e.g. ``dump-cluster-state``)
     - None
   * - ``--ohpc-packages <pkg>``
     - Provide a custom list of OpenHPC packages
     - Curated default set
   * - ``--roles <name[,name]>``
     - Run only the listed roles instead of the full workflow
     - Full workflow
   * - ``--list-roles``
     - Print all role identifiers supported by ``--roles`` and exit
     - Off
   * - ``-u`` / ``--unattended``
     - Suppress interactive confirmation prompts
     - Off
   * - ``--dump-answerfile <path>``
     - Render the in-memory model back to an answerfile at the given path
     - Off
   * - ``--config <path>``
     - Read additional CLI flags from a configuration file
     - Off
   * - ``--test <cmd>`` *(debug builds only)*
     - Execute a built-in integration test command and exit
     - Off
   * - ``--test-args <arg>`` *(debug builds only)*
     - Forward extra arguments to ``--test``
     - None

Repeat ``--skip`` or ``--force`` as needed, or supply comma-separated values.
Each token maps directly to an installer step.

Role-Driven Runs with ``--roles``
---------------------------------

Target specific workflows by enumerating role names:

.. code-block:: bash

   cloysterhpc -a some-answerfile.ini --roles base,network,ohpc -u

Role identifiers are case-insensitive and advertised by ``--list-roles``. They
unlock precise control over your deployment pipeline:

- ``check`` – Validates prerequisites such as pending kernel updates.
- ``repos`` – Seeds the operating system and CloysterHPC repositories.
- ``network`` – Configures NetworkManager profiles, hostnames and ``/etc/hosts``.
- ``sshd`` – Restricts root logins to key-based authentication.
- ``ofed`` – Installs InfiniBand support based on the ``[ofed]`` section. Combine
  with ``--skip infiniband`` or ``--force infiniband-install`` for fine control.
- ``dump`` – Captures pre-install system state for audits.
- ``locale`` – Applies the locale specified in ``[time]``.
- ``firewall`` – Enables or disables firewalld and tunes zone membership per
  network.
- ``selinux`` – Sets SELinux to enforcing, permissive or disabled according to
  your plan.
- ``nfs`` – Exports ``/opt/ohpc`` to compute nodes.
- ``queuesystem`` – Chooses SLURM or PBS logic; SLURM triggers the ``slurm`` role
  automatically.
- ``slurm`` – Installs and configures SLURM, MariaDB and generated queue
  definitions informed by the answerfile.
- ``ohpc`` – Installs the OpenHPC components (customisable with
  ``--ohpc-packages``).
- ``provisioner`` – Dispatches to either ``xcat`` or ``confluent`` roles.
- ``xcat`` – Installs and configures xCAT provisioning infrastructure.
- ``confluent`` – Installs Confluent provisioning services.
- ``base`` – Installs foundational packages, EPEL support and timezone
  configuration.
- ``audit`` – Deploys auditd with hardened defaults.
- ``aide`` – Installs AIDE and builds the initial integrity database.
- ``fail2ban`` – Activates fail2ban with opinionated SSH policies.
- ``timesync`` – Installs chrony and opens access for management/service
  networks.
- ``spack`` – Clones Spack into ``/opt/spack`` unless skipped.

Pair ``--roles`` with ``--skip``/``--force`` to engineer exactly the experience
your cluster build requires.
