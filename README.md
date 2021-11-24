# Cloyster HPC

Cloyster HPC is a software program to guide the user to set up an HPC clustered
environment. It asks for specific questions to get a basic HPC cluster up and
running as quick as possible. That's Cloyster HPC.

Its goal is to enhance the installation experience, making it user-friendly,
creating an easy-to-use questionnaire built with a user interface, that can be
swapped, for gathering and verifying the required data. For experienced users, 
an unattended mode is available with an .ini style configuration file.

# Documentation 

Documentation is not yet available, but the software usage should be
self-explanatory since help information is baked directly into the program.

# Architecture

Cloyster HPC is written in modern C++ and some leftover code in plain C with
[newt](https://pagure.io/newt) as the terminal user interface.

The software tries to implement an architectural pattern based on
[MVP](https://en.wikipedia.org/wiki/Model–view–presenter) (Model-View-Presenter)
where the components can be easily swapped in the future if needed.

There's no need for a database setting since all the configuration is stored in
a single [INI style](https://en.wikipedia.org/wiki/INI_file) configuration file.

## Features to be implemented

* Get an MVP (tm): Minimum Viable Product.
* Verify all user input against the OS, and it's libraries, for example, allow
dynamic population of network interfaces based on the hardware instead of just
asking the user for the data.
* Command line arguments for unattended installation.
* Wrap command execution for better control of states and not relying only on
"exit 0" status from the shell.
* Unattended answer file via .conf file on user homedir.
* Localization support.

## Dependencies

* Cloyster HPC requires [boost](https://www.boost.org) C++ libraries to be
installed for compilation.
* External [fmt](https://fmt.dev/latest/index.html) library is required while
std::format isn't available in major compilers.
* Logging is available through [spdlog](https://github.com/gabime/spdlog).

## Decisions

* Cloyster HPC uses C++20 as the programming languages. We are not looking
for memory and performance optimization here, the features that we want are a
flexible installer that can interact with the Operating System without much
hassle. We don't usually optimize for memory, so we avoid dynamic memory
allocation to keep the code as much memory safe as possible, even if this costs
system resources, which are pretty much abundant anyway on an HPC headnode.
* The newt library is chosen since it seems to be the default library to do
terminal composing on Linux systems, which is the goal here. We are basically
developing this with Red Hat Enterprise Linux, and it's clones in mind, and newt
is basically a Red Hat (and Debian) sponsored library. Finally, newt is more 
easy to work than ncurses, so it seems like a good fit.
* Static binaries with everything on it are possible, removing the need to
manually preinstall dependencies. Everything can be checked and installed by
the program, which enhances the user experience.
* We avoid C++ techniques such as multiple inheritance because in this context 
they are usually a **bad idea**, adding unnecessary complexity to what we are 
trying to achieve here. This may change in the future, but for now we're good
without abusing OOP paradigms.
* Why this isn't written in the __whatever__ language since it's more modern? 
-- _Simply because I tend to write better C++ and C code than other languages,
and the libraries and bindings that I need, and willing to use in the future,
were promptly available in C++ or C. Also C++ is very good at managing an OS
directly. We're aware that Python can do it too, but Python would create more
issues than C++ in this case._

## Status

| Cloyster Version | Build Status | 
|---|---|
| Development Release | N/A |
| Stable (latest release) | N/A |

## Building

We use CMake as building system for CloysterHPC. The CMake infrastructure was
heavily based on 
[cpp_starter_project](https://github.com/cpp-best-practices/cpp_starter_project)
with some modifications.

All default CMake targets are supported. We default to **Debug** target with
**DUMMY** option enabled to generate a dummy binary that does not modifies the
running OS.

To build the software just run the commands:

```
 cmake -S . -B ./build
 cmake --build ./build
```

Resulting binary will be available on `bin` directory in the root directory of
the project.

As a final warning: running the software without **DUMMY** option will probably
damage the running OS if they run as **root**. Be advised.

# Open Source Apache License

Cloyster HPC is made available under the Apache License, Version 2.0: 
https://www.apache.org/licenses/LICENSE-2.0

# Developers

Want to help? Feel free to open a discussion thread, so we can discuss features
and enhancements. After the first release we will start accepting pull requests
(PR) so you can get your code merged.

If you catch a bug please let us know! You know, malloc() isn't our friend...
