#!/bin/sh

# Create extra repositories file
cat <<EOL > /etc/yum.repos.d/extras.repo
[codeready-builder-for-rhel-9-x86_64-rpms]
name=CodeReady Builder for RHEL 9 x86_64
baseurl=https://mirror.versatushpc.com.br/rhel/codeready-builder-for-rhel-9-x86_64-rpms/
enabled=1
gpgcheck=1
gpgkey=https://mirror.versatushpc.com.br/rhel/RPM-GPG-KEY-redhat-release

[rhel-9-for-x86_64-appstream-rpms]
name=Appstream for RHEL 9 x86_64
baseurl=https://mirror.versatushpc.com.br/rhel/rhel-9-for-x86_64-appstream-rpms/
enabled=1
gpgcheck=1
gpgkey=https://mirror.versatushpc.com.br/rhel/RPM-GPG-KEY-redhat-release

[rhel-9-for-x86_64-baseos-rpms]
name=BaseOS for RHEL 9 x86_64
baseurl=https://mirror.versatushpc.com.br/rhel/rhel-9-for-x86_64-baseos-rpms/
enabled=1
gpgcheck=1
gpgkey=https://mirror.versatushpc.com.br/rhel/RPM-GPG-KEY-redhat-release

[OpenHPC]
name=OpenHPC-3 - Base
baseurl=http://repos.openhpc.community/OpenHPC/3/EL_9
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-OpenHPC-3
enabled=1

[OpenHPC-updates]
name=OpenHPC-3 - Updates
baseurl=http://repos.openhpc.community/OpenHPC/3/updates/EL_9
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-OpenHPC-3
enabled=1

EOL
echo "Extra repositories file created successfully."

dnf install -y libasan8 libubsan
