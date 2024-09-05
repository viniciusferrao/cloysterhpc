.. _tui-installation:

=========================================================
Installation Guide for Terminal Text-based User Interface
=========================================================

To set up Cloyster in your environment, you must first download the project in the head node.

For the current list of operating systems supported and minimum requirements, read section ":doc:`System Requirements <../../overview/sys_os_requirements>`"

Cloyster' TUI runs in a terminal, without an answerfile, with guided installation divided into different categories that will be explained in the subsection below.

Guided Installation
---------------------------

General Cluster settings
~~~~~~~~~~~~~~~~~~~~~~~~

The installation starts after this screen.
Press OK to start the installation.

.. image:: img/guided-installation/beginning.png
   :alt: Beginning of the installation phase

Here you must insert your cluster name, company name, and administrator email.

.. image:: img/guided-installation/clusterinfo.png
   :alt: Insert the cluster information

And you must select the boot target.

.. image:: img/guided-installation/boot-target.png
   :alt: Select the boot target

Time and clock settings
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Select your local timezone, searching through your continent and country/state.

.. image:: img/guided-installation/tzcontinent.png
   :alt: Choose your local timezone continent

.. image:: img/guided-installation/tzchoice.png
   :alt: Choose your local timezone

Select the time servers for your region, according to the available servers listed in `ntppool <https://www.ntppool.org>`_.

.. image:: img/guided-installation/timeserver.png
   :alt: Insert your time server

Locale settings
~~~~~~~~~~~~~~~~~

Select the appropriate default locale (used by programs or libraries for rendering text, correctly displaying regional
monetary values, time and date formats, alphabetic 	idiosyncrasies, and other locale-specific standards).

.. image:: img/guided-installation/locale.png
   :alt: Pick your default locale

Hostname settings
~~~~~~~~~~~~~~~~~~

Enter the machine hostname and domain name.

.. image:: img/guided-installation/hostname-domainname.png
   :alt: Insert the new cluster hostname and domain name

Network settings
~~~~~~~~~~~~~~~~~~

Fill in the information regarding the machine's external network.

.. image:: img/guided-installation/external-network.png
   :alt: Select your network interface

.. image:: img/guided-installation/external-networkinterface.png
   :alt: Choose the network interface

.. image:: img/guided-installation/external-networkdetails.png
   :alt: Insert the network details

Fill in the information regarding the machine's internal management network.

.. image:: img/guided-installation/management-network.png
   :alt: Select your network interface

If you have an available Infiniband Fabric, also fill in the information as done previously for other networks.

.. image:: img/guided-installation/infiniband-choice.png
   :alt: Infiniband information

Nodes operational system settings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For provisioning images to the computational nodes, choose if you want to download an ISO or use a local image:

.. image:: img/guided-installation/iso-choice-1.png
   :alt: Choose if you want to download an ISO or use a local image

If 'YES':
Select the desired operating system.

.. image:: img/guided-installation/iso-choice-2.png
   :alt: Choose a distro to download

If 'NO':
Provide the directory where the local ISO is located.

.. image:: img/guided-installation/iso-choice-3.png
   :alt: Inform the path to the directory where your operational system images are

Select the operating system of the ISO.

.. image:: img/guided-installation/iso-choice-4.png
   :alt: Choose a distro

Provide the name of the ISO file.

.. image:: img/guided-installation/iso-choice-5.png
   :alt: Choose an ISO

Compute nodes settings
~~~~~~~~~~~~~~~~~~~~~~~~

Press OK to fill your compute nodes data.

.. image:: img/guided-installation/compute-nodes.png
   :alt: Beginning of the compute nodes settings

Enter the information for the nodes as requested.

Examples:

- prefix=n, padding=2 (n01, n02, n03, nXX)
- Compute node first IP

    - n01: 172.26.0.1
    - n02: 172.26.0.2
    - nXX: 172.26.0.XX

.. image:: img/guided-installation/nodes-info.png
   :alt: Inform your nodes information

Enter each node MAC address.

.. image:: img/guided-installation/node-details.png
   :alt: Enter the MAC address for each node

Queue system settings
~~~~~~~~~~~~~~~~~~~~~~~~

Select the desired queue system (`SLURM <https://slurm.schedmd.com/>`_ or `PBS <https://www.openpbs.org/>`_).

.. image:: img/guided-installation/queue-system.png
   :alt: Choose a queue system

Mail system settings
~~~~~~~~~~~~~~~~~~~~~~~~

Choose if you want to enable Postfix mail system or no.

.. image:: img/guided-installation/enable-postfix.png
   :alt: Choose if you want to enable Postfix mail system
