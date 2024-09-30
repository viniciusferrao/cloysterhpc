.. _lvm-snapshot:

========================
Generate a LVM snapshot
========================

You can read the command list by running:

``./cloyster L -h``

The commands are straightforward and don't require any user input.

.. code-block:: text

    -a                          Check system availability for LVM snapshot.
    -c                          Create a LVM snapshot.
    -r                          Rollback to the LVM snapshot created.
    -d                          Destroy the LVM snapshot created.

.. note::
    We recommend creating the LVM snapshot before running Cloyster. After rolling back to a old snapshot, don't forget to reboot the machine.
