
# Contributing to CloysterHPC

You're very much welcome to contribute with this project.
Not only pushing code, but also testing it, improving the documentation and reporting bugs.
As a contributor, you can check the following guidelines:

## <a name="coc"></a> Code of Conduct

Just be respectful.
You can also check the exact same phrase at our single line Code of Conduct file.


## <a name="question"></a> Got a Question or Problem?

If you have any question or problem don't hesitate in opening a discussion.
We only ask that you do not open issues for support questions, since the issues tab is for specific software defects or missing functionality.


## <a name="issue"></a> Found a Bug?

If you find a bug in the source code, you can help us by [submitting an issue](#submit-issue).
You can also [submit a pull request](#submit-pr) with a proposed fix.
We will happily review your changes and merge it if it's a good fix.


## <a name="feature"></a> Missing a Feature?
You can ask for a new feature by [submitting an issue](#submit-issue) to our issue tracker.
If you aren't sure about this new feature, you can open a discussion first 
and if it makes sense we will add an issue to implement it.
Finally, if you would like to implement a new feature, we just ask to follow 
the guidelines for contributing to the project and add or fix the 
documentation if needed.

Your pull request will be greatly appreciated.


## <a name="rules"></a> Coding Rules
To ensure consistency throughout the source code, keep these rules in mind as you are working:

* The bare minimum for a PR is that the code **must** be able to compile on Enterprise Linux 8.
* English **must** be used as the primary language for the code.
* Documentation is **required** for features and bug fixes.
* License headers **must** be included in all files.
* We use [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to keep the coding style consistent.
``.clang-format`` file is shipped within the source, please use it.
* The code is written in modern C++ (specifically C++20) and is **not** using any deprecated features.
We will **not** support any deprecated features.
* External libraries are not only allowed but stimulated. Don't reinvent the wheel.
* Prefer STL functions over external libraries functions where is possible.
This happens a lot with [Boost](https://www.boost.org/).
* Remember to check [compiler compatibility](https://en.cppreference.com/w/cpp/compiler_support) for language features and library features.

## <a name="commit"></a> Commit Message Format

The following rules are required for commit messages:

* The commit message should have a quick and small one-line summary of the change on the header.
* If needed a more detailed description of the change should be included on the body of the commit message.
* All commits **must** be signed-off by the author on the footer: `git commit --signoff`.
* We recommend that your commits are signed with a GPG signature, since the project is running on vigilant mode.
* Reference issues or pull requests in the commit message if applicable.
* The skeleton of a commit message consists of a **header**, an optional **body**, and a **footer**, with blank lines between them:
    ```
    <header>
    <BLANK LINE>
    <optional body>
    <BLANK LINE>
    <footer>
    ```

    * Example commit message:
      ```
      Feature: Add support for RISC-V

      This feature adds support for RISC-V microprocessor architecture.
      Implements #387

      * Changes were made on the network, provisioner, OS, and server classes.
      * Extensive testing should be done, but compilation works on Pineapple ONE device.
      * Mixing architectures is not handled in this commit.

      Signed-off-by: Johnny Armless <johnny@armless.example.com>
      ```


## <a name="submit"></a> Submission Guidelines


### <a name="submit-issue"></a> Submitting an Issue

Before you submit an issue, we gently ask to search the existing list of 
issues, since your problem might already exist, and it's already being treated.

While submitting a new issue or contributing to an existing one, add all the 
relevant information to reproduce the issue and what you expected to happen. 
Don't be afraid to be too verbose.


### <a name="submit-pr"></a> Submitting a Pull Request (PR)

Before you submit your pull request follow the standard approach:

* Search for an existing similar pull request that relates to your submission. 
  If you find one, add your changes to the existing PR.

* Be sure to be descriptive regarding the problem you're fixing or the new functionality you're adding.
  If there's a need to update documentation due to your changes, please add the updates on the same pull request.
  Discussing the design upfront helps to ensure that we're ready to accept your work.

If everything is fine, you should proceed with your work:

* Fork this git repository.

* In your forked repository, make your changes in a new git branch, using a descriptive name:

     ```
     git checkout -b descriptive-branch-name master
     ```

* Do your modification in the created `descriptive-branch-name` branch, following our [Coding Rules](#rules).

* Add the changes to the branch:

     ```
     git add .
     ```

* Commit your changes using a descriptive commit message that follows our [commit message conventions](#commit).

     ```
     git commit --all --signoff
     ```

  * _Note_: the optional commit `-a` command line option will automatically "add" and "rm" edited files.

* Push your new branch:

    ```
    git push origin descriptive-branch-name
    ```

* Fill a pull request to the `master` branch.


### Reviewing a Pull Request

Your PR will be reviewed and accepted if it follows the guidelines.
We may reserve the right to accept or reject your PR at any time.


#### Addressing review feedback

If we ask for changes via code reviews then:

* Gently make the required updates to the code.

* Create a fixup commit and push to your repository, since this will update your pull request automatically:

    ```
    git commit --all --fixup HEAD
    git push
    ```

* The review process will restart, and hopefully we will accept your PR.


##### Updating the commit message

A reviewer might suggest changes to a commit message in some cases, like:

* Add more context for a given change
* Reference a specific issue or pull request
* Add a link to a relevant issue or pull request
* Fix typos in the commit message
* Enhance documentation regarding the code changes
* Adhere to our [commit message guidelines](#commit)

In order to update the commit message of the last commit on your branch:

1. Check out your branch:

    ```
    git checkout descriptive-branch-name
    ```

2. Amend the last commit and modify the commit message:

    ```
    git commit --amend
    ```

3. Push to your repository:

    ```
    git push --force-with-lease
    ```

> NOTE:<br />
> If you need to update the commit message of an earlier commit, you can use `git rebase` in interactive mode.
> See the [git docs](https://git-scm.com/docs/git-rebase#_interactive_mode) for more details.


#### After your pull request is merged

After your pull request is merged, you can safely delete your branch and pull the changes from the master (upstream) repository:

* Delete the remote branch directly on the web interface or either through your local shell.

Instructions for local shell removal:

* Delete the local branch:

    ```
    git push origin --delete descriptive-branch-name
    ```

* Check out the `master` branch:

    ```
    git checkout master -f
    ```

* Delete the local branch:

    ```
    git branch -D descriptive-branch-name
    ```

* Update your local `master` with the latest upstream version:

    ```
    git pull --ff upstream master
    ```


## <a name="thanks"></a> Thanks

Thank you for contributing with this project. We hope you enjoy it!
