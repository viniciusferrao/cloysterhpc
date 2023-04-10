# CloysterHPC

CloysterHPC is a software that guides the user to set up
an [HPC](https://en.wikipedia.org/wiki/High-performance_computing) clustered
environment. It asks for specific questions regarding the system to get an 
HPC cluster up and running as quick as possible. That's CloysterHPC.

Its goal is to enhance the installation and maintenance experience, making it
user-friendly, creating an easy-to-use questionnaire built with a familiar
user interface, for gathering and verifying the required cluster information.
For experienced users, an unattended mode will be available in future releases
with a classic configuration file.

# User documentation

Documentation about usage is not yet available, but the software
should be self-explanatory since help information is baked directly into it.

# Architecture

CloysterHPC is written in modern C++ with some C code for compatibility
with
[newt](https://pagure.io/newt), that's used as the terminal user interface.
The software follows the Object-Oriented Paradigm and implements a simplified
architectural pattern based on
[MVP](https://en.wikipedia.org/wiki/Model–view–presenter) (Model-View-Presenter)
where the components can be easily swapped or expanded in the future if
needed.

## Major features to be implemented

* Command line arguments for unattended installation.
* Unattended answer file via `.conf` file on user homedir or predefined
  location.
* Localization support.
* Always running daemon to ease cluster maintenance.
* Static binaries.

## Dependencies

* CloysterHPC requires [boost](https://www.boost.org) C++ libraries.
* External [fmt](https://fmt.dev/latest/index.html) library is required while
  `std::format` isn't available in major compilers.
* Logging is available through [spdlog](https://github.com/gabime/spdlog).
* [magic_enum](https://github.com/Neargye/magic_enum) is used for static `enum`
  reflections.
* Adherence with best practices is done
  with [gsl-lite](https://github.com/gsl-lite/gsl-lite).
* [newt](https://pagure.io/newt) for Terminal UI.

Only [newt](https://pagure.io/newt) must be pre-installed for compilation. We
don't ship it. Everything else should be found and installed
by [Conan](http://conan.io) during
[CMake](https://cmake.org). Our shipped build script should handle this
automatic.

## Decisions

* CloysterHPC uses C++20 as the programming languages. We are not looking
  for memory and performance optimization here, we strive for correctness. The
  features that we want are a
  flexible software that can interact with the Operating System without much
  hassle. We don't prematurely optimize for resources, but we avoid unnecessary
  dynamic memory
  allocations even if this
  costs some system resources, which are pretty much abundant anyway on an
  HPC system manager (the headnode).
* The [newt](https://pagure.io/newt) library is chosen since it seems to be the
  default library to do
  terminal composing on Linux systems, which is the goal here. We are basically
  developing this with Red Hat Enterprise Linux, and it's clones in mind, and
  [newt](https://pagure.io/newt)
  is basically a Red Hat (and Debian) sponsored library.
  Finally, [newt](https://pagure.io/newt) is easier to work
  than [ncurses](https://invisible-island.net/ncurses), so it seems
  like a good fit.
* We avoid OOP techniques that may add complexity without any visible
  benefits, such as multiple inheritance. Because in this context
  they are usually a **bad idea**. It's not forbidden to use, but it should
  be used with caution.
* At the current state there's no need for a database since the software is
  stateless, and in the future configuration will be stored in a single [INI
  style](https://en.wikipedia.org/wiki/INI_file) or equivalent configuration
  file.
* Build system is based on [CMake](https://cmake.org)
  and [Conan](http://conan.io) for package management. It was
  heavily based on
  [cpp_starter_project](https://github.com/cpp-best-practices/cpp_starter_project)
  with some modifications.
* Expansion and modularity is desired and should be achieved. A daemon
  compatible with `systemd` is planned to enable more powerful features like:
  keeping state,
  maintenance interface, common housekeeping routines, backups and upgrades.
* We consume a lot of existing software to avoid recreating everything from
  scratch, most evidently: [xCAT](https://xcat.org)
  and [OpenHPC](http://openhpc.community). We also push changes on consumed
  projects to enhance them, so we directly benefit from those changes.
* CloysterHPC is not made to run inside a container. It needs _bare metal_
  access.
* Why this isn't written in the __whatever__ language since it's more modern?
  -- _Simply because I tend to write better C++ and C code than other languages,
  and the libraries and bindings that I need, and willing to use in the future,
  were promptly available in C++ or C. Also C++ is very good at managing an OS
  directly. We're aware that Python can do it too, but Python would create more
  issues than C++ in this case._

## Status

CloysterHPC is alpha quality software. Some features are still missing, but
they are on the roadmap. Production use should be done with caution and with
preliminar testing.

## Building

We recommend to build and run this software in a virtual machine due to its
nature and `root` execution requirements.

## Running tests

To run tests you need to add the `-DENABLE_TESTING:BOOL=ON` option to CMake to build it.

### Recommended VM settings

* EL8 based system with **minimal** package selection
* Half of system CPU cores as vCPU
* At least 4GB of RAM
* 50GB of Disk
* 2 Network Interfaces
    * External Access
    * Internal Management Network
* Disable side channel mitigations (for performance reasons)
* UEFI mode

To compile and run the software, clone this repository and execute the
`setupDevEnvironment.sh` shell
script.
All dependencies should be installed after its execution and then follow
with standard [CMake](https://cmake.org) procedure.

All default [CMake](https://cmake.org) targets are supported. By default, it
builds a **Debug** target with sanitizers enabled that modifies the
running OS, so be **advised to run the code on a development system only**.

Finally, to build the software just run the commands:

```
 $ git submodule init
 $ git submodule update
 $ cmake -S . -B ./build
 $ cmake --build ./build
```

Resulting binary will be available on `bin` directory in the root directory of
the project.

As a final warning: running the software without **DUMMY** option will probably
damage the running OS if they run as **root**. Be advised.

# Open Source Apache License

CloysterHPC is made available under
the [Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0).

# Developers

Want to help? Feel free to report bugs, issues or open a discussion thread,
so we can discuss features and enhancements. Pull requests are greatly
appreciated.

# Not a developer?

Any feedback is welcome. Feel free to use it, report issues and complain about
the software.
