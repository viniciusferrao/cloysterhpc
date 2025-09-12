#!/bin/bash
#
# Setup the internet network interface with NetworkManager: nmtui
# ens192: connected to ens192
#         "VMware VMXNET3"
#         ethernet (vmxnet3), 00:0C:29:DD:6F:F6, hw, mtu 1500
#         inet4 100.64.0.254/24
#         route4 100.64.0.0/24 metric 102
#         inet6 fe80::20c:29ff:fedd:6ff6/64
#         route6 fe80::/64 metric 1024

systemctl disable --now firewalld
sed -e 's/^SELINUX=.*/SELINUX=disabled/' -i /etc/selinux/config
setenforce 0

hostname=cloyster
domain=cluster.example.com
hn_ip=192.168.30.254
cidr=24
external_iface=enp2s2
conn_name=Management
nameservers=192.168.122.1
gateway=192.168.122.1

nodes_mac=()
nodes_ip=()

nodes_mac[0]=ca:fe:de:ad:be:ef
nodes_ip[0]=192.168.30.1

iso_path=/opt/iso/Rocky-9.6-x86_64-dvd.iso
image=rocky-9.6-x86_64

hostnamectl set-hostname ${hostname}.${domain}
grep -q "${hn_ip} ${hostname}.${domain} ${hostname}" /etc/hosts || echo "${hn_ip} ${hostname}.${domain} ${hostname}" >> /etc/hosts
cat <<EOF > /etc/NetworkManager/conf.d/90-dns-none.conf
[main]
dns=none
EOF
systemctl restart NetworkManager

nmcli device set ${external_iface} managed yes
nmcli device set ${external_iface} autoconnect yes

# Remove existing connection if present
nmcli con show ${external_iface} > /dev/null && nmcli conn delete ${external_iface}
nmcli con show ${conn_name} > /dev/null && nmcli connection delete ${conn_name}

# Add new static IPv4 + link-local IPv6 connection
nmcli connection add type ethernet mtu 1500 ifname ${external_iface} con-name ${conn_name} \
    ipv4.method manual \
    ipv4.addresses "${hn_ip}/${cidr}" \
    ipv4.gateway "" \
    ipv4.ignore-auto-dns yes \
    ipv4.ignore-auto-routes yes \
    ipv6.method link-local

sleep 1
nmcli -w 10 device connect ${external_iface}

# Add the Confluent repository
# Latest
rpm -ivh https://hpc.lenovo.com/yum/latest/el9/x86_64/lenovo-hpc-yum-1-1.x86_64.rpm
#
# Non-latest
# wget https://hpc.lenovo.com/downloads/25.6/confluent-3.13.3-el9.tar.xz \
#  && tar xf confluent-3.13.3-el9.tar.xz \
#  && cd lenovo-hpc-el9/ \
#  && ./mklocalrepo.sh

# Install required packages
# Technically only `lenovo-confluent` is requqired, however: 
# 1. If we are dealing with legacy systems we may need `tftp-server` also
# 2. If we want DNS resolution, which we may want, also add `dnsmasq`.
dnf install -y lenovo-confluent tftp-server dnsmasq
systemctl enable confluent --now
systemctl enable httpd --now 
systemctl enable tftp.socket --now
systemctl enable dnsmasq --now

# Enable the Confluent environment (or just relogin if you prefer)
source /etc/profile.d/confluent_env.sh

# Configure SELinux to allow httpd to make connections
# setsebool -P httpd_can_network_connect=on

# Configure the internal network interfaces as trusted on FirewallD
# firewall-cmd --zone=trusted --change-interface=ens192 --permanent
# firewall-cmd --reload

# Add basic settings to allow a minimalist boot environment for testing
nodegroupattrib everything dns.servers=$hn_ip dns.domain=$domain net.ipv4_gateway=$hn_ip \
    deployment.useinsecureprotocols=always

# Fill required passwords
# FIXME: Those strings are get from `stdin` we need to automate also
# nodegroupattrib everything -p bmcuser bmcpass crypted.rootpassword crypted.grubpassword
nodegroupattrib everything bmcuser=admin bmcpass=admin crypted.rootpassword=admin crypted.grubpassword=admin

# Generate a keypair for internal cluster usage
# This command may issue prompts, the <<< n is to not replace
# the old ssh key
ssh-keygen -f ~/.ssh/id_ed25519 -t ed25519 -N "" <<< n

# Define a given node
nodedefine n01
nodeattrib n01 net.hwaddr=${nodes_mac[0]}
nodeattrib n01 net.ipv4_address=${nodes_ip[0]}/${cidr}

# Add nodes to /etc/hosts
# Needs to evaluate in the future if there is a better approach to add files to /etc/hosts; `dnsmasq` maybe?
confluent2hosts -a everything

# Configure the osdeploy parameters
osdeploy initialize -u -s -k -l -p -a -t -g

# Import the OS ISO file.
osdeploy import $iso_path 

# Create a temporary chroot to work as basis for the boot image
rm -rf /tmp/scratchdir || :
rm -rf /var/lib/confluent/public/os/${image}-diskless || :
imgutil build -y -s $image /tmp/scratchdir
imgutil pack /tmp/scratchdir/ $image-diskless
# Check if the image shows up as available with the defined name
osdeploy list


# Remove the leftover files from the chroot
rm -rf /tmp/scratchdir

# Configure a given node to be stateless
# -p flag is to ignore BMC commands (if you are running in a system without BMC)
# -n Network boot
nodedeploy -p n01 -n $image-diskless

tail -F /var/log/confluent/{events,audit} /var/log/httpd/*_log
# Reboot n01 and watch the boot process through its monitor (or KVM) and check
# the logs in the headnode


# Check if node is booted and acessible:
ssh n01

############################

# References:
# <https://hpc.lenovo.com/users/documentation/confluentquickstart_el8.html>
# <https://hpc.lenovo.com/users/documentation/installconfluent_rhel.html>
# <https://hpc.lenovo.com/users/documentation/confluentdiskless.html>
# <https://hpc.lenovo.com/users/documentation/confluentosdeploy.html>
# 
# # Debug Commands
# tcpdump -i ens192 # Internal network
# tail -f /var/log/confluent/audit /var/log/confluent/events /var/log/confluent/stdout /var/log/httpd/*
