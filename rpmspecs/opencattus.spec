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
mkdir -p %{buildroot}/opt/cloysterhpc/conf/repos/
install -m 755 build/src/cloysterhpc %{buildroot}/usr/bin/cloysterhpc
install -m 755 repos/repos.conf %{buildroot}/opt/cloysterhpc/conf/repos/repos.conf
install -m 755 repos/alma.conf %{buildroot}/opt/cloysterhpc/conf/repos/alma.conf
install -m 755 repos/rhel.conf %{buildroot}/opt/cloysterhpc/conf/repos/rhel.conf
install -m 755 repos/oracle.conf %{buildroot}/opt/cloysterhpc/conf/repos/oracle.conf
install -m 755 repos/rocky-upstream.conf %{buildroot}/opt/cloysterhpc/conf/repos/rocky-upstream.conf
install -m 755 repos/rocky-vault.conf %{buildroot}/opt/cloysterhpc/conf/repos/rocky-vault.conf

%files
/usr/bin/cloysterhpc
/opt/cloysterhpc/conf/repos.conf

%changelog
* Tue Jun 10 2025 Daniel Hilst <daniel@versatushpc.com.br> - 1.0-2
- Repositories revamped
- NFS fixed
- Doca OFED support added
* Tue Feb 25 2025 Daniel Hilst <daniel@versatushpc.com.br> - 1.0-1
- Initial release
