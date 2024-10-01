.. _submission-guidelines:

=====================
Submission Guidelines
=====================

Coding Rules
------------

To ensure consistency throughout the source code, keep these rules in mind as you are working:

- The bare minimum for a PR is that the code **must** be able to compile on Enterprise Linux 8.
- English **must** be used as the primary language for the code.
- Documentation is **required** for features and bug fixes.
- License headers **must** be included in all files.
- We use `clang-format` to keep the coding style consistent.
    - ``.clang-format`` file is shipped within the source, please use it.
- The code is written in modern C++ (specifically C++23) and is **not** using any deprecated features.
    - We will **not** support any deprecated features.
- External libraries are not only allowed but stimulated. Don't reinvent the wheel.
- Prefer STL functions over external libraries functions where is possible.
    - This happens a lot with `Boost`.
- Remember to check `compiler compatibility <https://en.cppreference.com/w/cpp/compiler_support>`_ for language features and library features.

Commit Message Format
-----------------------

The following rules are required for commit messages:

- The commit message should have a quick and small one-line summary of the change on the header.
- If needed a more detailed description of the change should be included on the body of the commit message.
- All commits **must** be signed-off by the author on the footer: `git commit --signoff`.
- We recommend that your commits are signed with a GPG signature, since the project is running on vigilant mode.
- Reference issues or pull requests in the commit message if applicable.
- The skeleton of a commit message consists of a **header**, an optional **body**, and a **footer**, with blank lines between them:

.. code-block:: text
   :caption: Example commit message

      Feature: Add support for RISC-V
      This feature adds support for RISC-V microprocessor architecture.
      Implements #387
      * Changes were made on the network, provisioner, OS, and server classes.
      * Extensive testing should be done, but compilation works on Pineapple ONE device.
      * Mixing architectures is not handled in this commit.
      Signed-off-by: Johnny Armless <johnny@armless.example.com>

Submitting an Issue
--------------------

Before you submit an issue, we gently ask to search the existing list of
issues, since your problem might already exist, and it's already being treated.

While submitting a new issue or contributing to an existing one, add all the
relevant information to reproduce the issue and what you expected to happen.
Don't be afraid to be too verbose.


Submitting a Pull Request (PR)
-------------------------------

Before you submit your pull request follow the standard approach:

- Search for an existing similar pull request that relates to your submission.
  If you find one, add your changes to the existing PR.

- Be sure to be descriptive regarding the problem you're fixing or the new functionality you're adding.
  If there's a need to update documentation due to your changes, please add the updates on the same pull request.
  Discussing the design upfront helps to ensure that we're ready to accept your work.

If everything is fine, you should proceed with your work:

- Fork this git repository.

- In your forked repository, make your changes in a new git branch, using a descriptive name:

     ``git checkout -b descriptive-branch-name master``

- Do your modification in the created `descriptive-branch-name` branch, following our Coding Rules.

- Add the changes to the branch:

      ``git add .``

- Commit your changes using a descriptive commit message that follows our commit message conventions.

     ``git commit --all --signoff``

  - _Note_: the optional commit ``-a`` command line option will automatically "add" and "rm" edited files.

- Push your new branch:

    ``git push origin descriptive-branch-name``

- Fill a pull request to the `master` branch.

Reviewing a Pull Request
-------------------------

Your PR will be reviewed and accepted if it follows the guidelines.
We may reserve the right to accept or reject your PR at any time.


Addressing review feedback
--------------------------

If we ask for changes via code reviews then:

- Gently make the required updates to the code.

- Create a fixup commit and push to your repository, since this will update your pull request automatically:

   ``git commit --all --fixup HEAD && git push``

- The review process will restart, and hopefully we will accept your PR.


Updating the commit message
---------------------------

A reviewer might suggest changes to a commit message in some cases, like:

- Add more context for a given change
- Reference a specific issue or pull request
- Add a link to a relevant issue or pull request
- Fix typos in the commit message
- Enhance documentation regarding the code changes
- Adhere to our commit message guidelines

In order to update the commit message of the last commit on your branch:

1. Check out your branch:

    ``git checkout descriptive-branch-name``

2. Amend the last commit and modify the commit message:

    ``git commit --amend``

3. Push to your repository:

   ``git push --force-with-lease``

> NOTE:<br />
> If you need to update the commit message of an earlier commit, you can use ``git rebase`` in interactive mode.
> See the `git docs <https://git-scm.com/docs/git-rebase#_interactive_mode>`_ for more details.

After your pull request is merged
----------------------------------

After your pull request is merged, you can safely delete your branch and pull the changes from the master (upstream) repository:

- Delete the remote branch directly on the web interface or either through your local shell.

Instructions for local shell removal:

- Delete the local branch:

    ``git push origin --delete descriptive-branch-name``

- Check out the `master` branch:

    ``git checkout master -f``

- Delete the local branch:

    ``git branch -D descriptive-branch-name``

- Update your local `master` with the latest upstream version:

    ``git pull --ff upstream master``
