Name: opencattus-installer 
Version: 1.0
Release: 1
Summary: OpenCATTUS Installer
License: Apache 2.0
URL: https://versatushpc.com.br/opencattus/
Source0: opencattus-%{VERSION}.tar.gz
BuildRequires: make,cmake,cppcheck,ninja-build,newt-devel,gcc-toolset-14,gcc-toolset-14-libubsan-devel,gcc-toolset-14-libasan-devel
Requires: newt

# Disable debug package for now
%global _enable_debug_package 0
%global debug_package %{nil}

%description
Use OpenCATTUS installer to setup a HPC cluster from scratch.

%prep
echo "PREP: $PWD"
%autosetup -n opencattus-%{VERSION}
bash -c '
	source rhel-gcc-toolset-14.sh;
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
'

%build
echo "BUILD: $PWD"
cmake --build build

%install
echo "INSTALL: $PWD"
mkdir -p %{buildroot}/usr/bin
install -m 755 build/src/cloysterhpc %{buildroot}/usr/bin/cloysterhpc

%files
/usr/bin/cloysterhpc

%changelog
* Tue Feb 25 2025 Daniel Hilst <daniel@versatushpc.com.br> - 1.0-1
- Initial release
