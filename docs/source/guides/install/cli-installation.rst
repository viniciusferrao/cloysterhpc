.. _cli-installation:

=============================================
Installation Guide for Command Line Interface
=============================================

CloysterHPC ships as a single binary optimized for unattended rollouts. Follow
these quick steps to launch a build from the command line:

1. Review the :ref:`Answerfile Reference <answerfile-reference>` to tailor the
   cluster blueprint for your environment.
2. Copy the answerfile to the head node alongside the ``cloysterhpc`` binary.
3. Execute the installer with elevated privileges:

   .. code-block:: bash

      cloysterhpc -a /path/to/answerfile.ini -l6 -u

   ``-a`` reads your blueprint, ``-l6`` increases log verbosity during the run,
   and ``-u`` skips confirmation prompts so the deployment can proceed
   unattended.

For an overview of every CLI flag or to fine-tune specific roles, jump to the
:doc:`CloysterHPC Operations Guide <../../cli_reference>`.
