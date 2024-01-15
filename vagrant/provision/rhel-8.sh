#!/bin/sh

# Create codeready repository file
cat <<EOL > /etc/yum.repos.d/codeready.repo
[codeready-builder-for-rhel-8-x86_64-rpms]
name=CodeReady Builder for RHEL 8 x86_64
baseurl=https://mirror.versatushpc.com.br/rhel/codeready-builder-for-rhel-8-x86_64-rpms/
enabled=1
gpgcheck=1
gpgkey=@TODO
EOL
echo "codeready-builder-for-rhel-8-x86_64-rpms repository file created successfully."

dnf install -y libasan8 libubsan
