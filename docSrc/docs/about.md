# About

CloysterHPC is a software that guides the user to set up an HPC clustered environment. It asks for specific questions regarding the system to get an HPC cluster up and running as quick as possible. That's CloysterHPC.

Its goal is to enhance the installation and maintenance experience, making it user-friendly, creating an easy-to-use questionnaire built with a familiar user interface, for gathering and verifying the required cluster information. For experienced users, an unattended mode will be available in future releases with a classic configuration file.

---

## Architecture

CloysterHPC is written in modern C++ with some C code for compatibility with newt, that's used as the terminal user interface. The software follows the Object-Oriented Paradigm and implements a simplified architectural pattern based on MVP (Model-View-Presenter) where the components can be easily swapped or expanded in the future if needed.

---

## Decisions

*   CloysterHPC uses C++20 as the programming languages. We are not looking for memory and performance optimization here, we strive for correctness. The features that we want are a flexible software that can interact with the Operating System without much hassle. We don't prematurely optimize for resources, but we avoid unnecessary dynamic memory allocations even if this costs some system resources, which are pretty much abundant anyway on an HPC system manager (the headnode).
*   The newt library is chosen since it seems to be the default library to do terminal composing on Linux systems, which is the goal here. We are basically developing this with Red Hat Enterprise Linux, and it's clones in mind, and newt is basically a Red Hat (and Debian) sponsored library. Finally, newt is easier to work than ncurses, so it seems like a good fit.
*   We avoid OOP techniques that may add complexity without any visible benefits, such as multiple inheritance. Because in this context they are usually a bad idea. It's not forbidden to use, but it should be used with caution.
*   At the current state there's no need for a database since the software is stateless, and in the future configuration will be stored in a single INI style or equivalent configuration file.
*   Build system is based on CMake and Conan for package management. It was heavily based on cpp_starter_project with some modifications.
*   Expansion and modularity is desired and should be achieved. A daemon compatible with systemd is planned to enable more powerful features like: keeping state, maintenance interface, common housekeeping routines, backups and upgrades.
*   We consume a lot of existing software to avoid recreating everything from scratch, most evidently: xCAT and OpenHPC. We also push changes on consumed projects to enhance them, so we directly benefit from those changes.
*   CloysterHPC is not made to run inside a container. It needs bare metal access.
*   Why this isn't written in the whatever language since it's more modern? -- Simply because I tend to write better C++ and C code than other languages, and the libraries and bindings that I need, and willing to use in the future, were promptly available in C++ or C. Also C++ is very good at managing an OS directly. We're aware that Python can do it too, but Python would create more issues than C++ in this case.

---

## Open Source Apache License

CloysterHPC is made available under the [Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0).