# Sync the clocks, I need this after restoring libvirt snapshots
systemctl restart chronyd
sleep 10
date

# Update the kernel, this is required to fix the error
# doca-kernel-support: Error: Could not determine kernel version from source
# directory /lib/modules/5.14.0-570.17.1.el9_6.x86_64/build
# This happens because the ISO kernel is not the same as the repository kernel
dnf install -y kernel

# Configure limits.conf
grep -qE '^* soft memlock unlimited' /etc/security/limits.conf || \
    echo '* soft memlock unlimited' >> /etc/security/limits.conf
grep -qE '^* hard memlock unlimited' /etc/security/limits.conf || \
    echo '* hard memlock unlimited' >> /etc/security/limits.conf

# Reboot the new kernel and new limits
reboot

# Now we're ready to go

# Export is required otherwise the variables are not visible
# during image customization
export hostname=cloyster
export domain=cluster.example.com
export hn_ip=192.168.30.254
export cidr=24
export internal_iface=enp2s2
export conn_name=Management
export management_network=192.168.122.0
 
# OFED
export ofed_enabled=true

# Slurm cofiguration
export cluster_name=cluster
export cpus_per_node=2
export real_memory=4096
export partition_name=batchp
export cores_per_socket=2
export threads_per_core=1
export storage_pass=xxxxxx # slurmdbd.conf StoragePass 
export mariadb_root_pass=xxxxxx # mariadb root password
export slurmdb_pass=xxxxxx # mariadb slurm database password

# Confluent Nodes configuration
export bmcuser=xxxxxx
export bmcpass=xxxxxx
export rootpass=xxxxxx
export grubpass=xxxxxx

export nodes_mac=()
export nodes_ip=()

nodes_mac[0]=ca:fe:de:ad:be:ef
nodes_ip[0]=192.168.30.1

# Confluent remaining config
export iso_path=/opt/iso/Rocky-9.6-x86_64-dvd.iso
export image=rocky-9.6-x86_64
export scratchdir=/tmp/scratchdir

# Enable Rocky Linux Vault, usage: enable_vault 9.5
enable_vault() {
  local release="$1" # e.g. 9.5
  echo "Rewriting .repo files to use Rocky Linux Vault repository"
  for file in /etc/yum.repos.d/rocky*.repo; do
    sed -i \
      -e '/^mirrorlist=/ s/^/#/' \
      -e '/^#baseurl=/ s/^#//' \
      -e "/^baseurl=/ s|\$contentdir|vault/rocky|g" \
      -e "/^baseurl=/ s|pub/rocky|vault/rocky|g" \
      -e "s|\$releasever|${release}|g" \
      "$file"
  done
}

# Enable bash debug
set -xeuo pipefail
# To disable run:
# set +xeu

# Disable firewall and SELinux
systemctl disable --now firewalld
sed -e 's/^SELINUX=.*/SELINUX=disabled/' -i /etc/selinux/config
setenforce 0

# Enable DNF cache. This is useful when you have to reinstall a package
# and don't want to download it again, not recommended for production
dnf config-manager --setopt=keepcache=1 --save

# Network configuration
hostnamectl set-hostname ${hostname}.${domain}
grep -q "${hn_ip} ${hostname}.${domain} ${hostname}" /etc/hosts || echo "${hn_ip} ${hostname}.${domain} ${hostname}" >> /etc/hosts
cat <<EOF > /etc/NetworkManager/conf.d/90-dns-none.conf
[main]
dns=none
EOF
systemctl restart NetworkManager

nmcli device set ${internal_iface} managed yes
nmcli device set ${internal_iface} autoconnect yes

# Remove existing connection if present
nmcli con show ${internal_iface} > /dev/null && nmcli conn delete ${internal_iface}
nmcli con show ${conn_name} > /dev/null && nmcli connection delete ${conn_name}

# Add new static IPv4 + link-local IPv6 connection
nmcli connection add type ethernet mtu 1500 ifname ${internal_iface} con-name ${conn_name} \
    ipv4.method manual \
    ipv4.addresses "${hn_ip}/${cidr}" \
    ipv4.gateway "" \
    ipv4.ignore-auto-dns yes \
    ipv4.ignore-auto-routes yes \
    ipv6.method link-local

sleep 1
nmcli -w 10 device connect ${internal_iface}
ip a show dev ${internal_iface}

# Chrony configuration
dnf install -y chrony
sed -e '/^allow/d' -i /etc/chrony.conf
echo "allow ${management_network}/${cidr}" >> /etc/chrony.conf
grep -H allow /etc/chrony.conf
systemctl restart chronyd

# OFED install
if ${ofed_enabled}; then
    cat <<EOF > /etc/yum.repos.d/doca.repo
[doca]
name=DOCA latest for RHEL 9.6
baseurl=https://linux.mellanox.com/public/repo/doca/latest-2.9-LTS/rhel9.6/x86_64/
enabled=1
gpgcheck=1
gpgkey=https://linux.mellanox.com/public/repo/doca/latest-2.9-LTS/rhel9.6/x86_64/GPG-KEY-Mellanox.pub
EOF
    dnf makecache --repo=doca
    # --nogpg mitigate
    # Package doca-extra-0.1.7-1.el9.noarch.rpm is not signed
    # Error: GPG check FAILED
    dnf install --nogpg -y kernel kernel-devel doca-extra tar
    # grub2-probe: error: ../grub-core/kern/disk.c:240:disk
    # `lvmid/V8Hf6U-WDTh-oSNh-Mnov-0Ozu-IWmW-EVwq0m/PlezqA-ZVTx-J8Qu-Y61f-prJB-YMuu-zl5M0i'
    # not found.

    /opt/mellanox/doca/tools/doca-kernel-support

    # Get the last RPM in /tmp/DOCA*/ folder
    docarpm=$(find /tmp/DOCA*/ -name '*.rpm' -printf '%T@ %p\n' | sort -nk1 | tail -1 | awk '{print $2}')
    dnf install -y $docarpm
    dnf makecache --repo='doca*'
    dnf install --nogpg -y doca-ofed mlnx-fw-updater
    systemctl restart openibd
    echo "openibd is $(systemctl is-active openibd)"

    # WARNING: This happend if NFS is already configured. NFS will pull modules that OFED
    # does not like and it will break it. Install OFED *BEFORE* NFS. If this happens you have
    # to stop nfs-server and rpcbind, unload rpcrdma and restart openibd
    # 
    # Not tested:
    # systemctl stop rpcbind nfs-server
    # modprobe -vr rpcrdma
    # systemctl start openibd 
    #
    # openibd: start(): Detected loaded old version of module 'rdma_cm', calling stop...
    # Please make sure module 'rpcrdma' is not in use and unload it.
    # Error: Cannot unload the Infiniband driver stack due to the above issue(s)!
    # To unload the blocking modules, you can run:
    # # modprobe -rv  rpcrdma
    # Once the above issue(s) resolved, run:
    # # /etc/init.d/openibd start
    # openibd.service: Main process exited, code=exited, status=1/FAILURE
fi

# Install EPEL
dnf install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm

# Install OHPC
dnf install -y http://repos.openhpc.community/OpenHPC/3/EL_9/x86_64/ohpc-release-3-1.el9.x86_64.rpm
dnf install -y ohpc-base

# NFS configuration
dnf install -y nfs-utils
echo > /etc/exports
echo "/home *(rw,no_subtree_check,fsid=10,no_root_squash)" >> /etc/exports
echo "/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)" >> /etc/exports
echo "/opt/spack *(ro)" >> /etc/exports

systemctl enable --now rpcbind nfs-server
if systemctl is-enabled --quiet firewalld.service; then
    # Untested
    firewall-cmd --permanent --add-service={nfs,mountd,rpc-bind}
    firewall-cmd --reload
fi
echo "rpcbind is $(systemctl is-active rpcbind)"
echo "nfs-server is $(systemctl is-active nfs-server)"

# SLURM configuration
dnf -y install ohpc-slurm-server mariadb-server mariadb
systemctl enable --now munge slurmctld munge slurmdbd mariadb

# Secure the installation, `mysql -u root` will exit with
# non-zero exit code if this already run before, use `mysql -u root -p`
# to login in the database
#
# If you need to wipeout the database and start over:
# systemctl stop mariadb
# systemctl disable mariadb
# dnf remove -y mariadb mariadb-server mariadb-libs
# rm -rf /var/lib/mysql
# rm -f /var/log/mariadb/*
# rm -f /etc/my.cnf
# rm -rf /etc/my.cnf.d
mysql -u root --password='' -e "SELECT 1;" >/dev/null 2>&1
if [ $? -eq 0 ]; then
    mysql -u root 2> /dev/null <<EOF
    ALTER USER IF EXISTS 'root'@'localhost' IDENTIFIED BY '$mariadb_root_pass';

    DELETE FROM mysql.user WHERE User='';
    DELETE FROM mysql.user WHERE User='root' AND Host NOT IN ('localhost', '127.0.0.1', '::1');

    DROP DATABASE IF EXISTS test;
    DELETE FROM mysql.db WHERE Db='test' OR Db='test_%';

    CREATE DATABASE IF NOT EXISTS slurm_acct_db;

    CREATE USER IF NOT EXISTS 'slurm'@'localhost' IDENTIFIED BY '$slurmdb_pass';
    GRANT ALL PRIVILEGES ON slurm_acct_db.* TO 'slurm'@'localhost';

    FLUSH PRIVILEGES;
EOF
fi

cat <<EOF > /etc/slurm/slurmdbd.conf
# Authentication
AuthType=auth/munge

# SLURM DB daemon identity
DbdHost=${hostname}
DbdPort=6819
SlurmUser=slurm

# Database storage
StorageType=accounting_storage/mysql
StorageHost=localhost
StorageUser=slurm
StoragePass=${storage_pass}
StorageLoc=slurm_acct_db

# Logging & PID
LogFile=/var/log/slurmdbd.log
PidFile=/var/run/slurmdbd.pid
EOF
chown slurm:slurm /etc/slurm/slurmdbd.conf
chmod 600 /etc/slurm/slurmdbd.conf

# Minimal /etc/slurm/slurm.conf
slurm_conf=/etc/slurm/slurm.conf
\cp /etc/slurm/slurm.conf.ohpc $slurm_conf

sed -i \
  -e "s/ClusterName=.*/ClusterName=${cluster_name}/" \
  -e "s/SlurmctldHost=.*/SlurmctldHost=${hostname}/" \
  -e "s|SlurmctldLogFile=.*|SlurmctldLogFile=/var/log/slurm/slurmctld.log|" \
  -e "s|SlurmdLogFile=.*|SlurmdLogFile=/var/log/slurm/slurmd.log|" \
  "$slurm_conf"

sed -i \
    -e '/^NodeName=/d' \
    -e '/^PartitionName=/d' \
    -e '/^AccountingStorageType=/d' \
    -e '/^AccountingStorageHost=/d' \
    -e '/^AccountingStoragePort=/d' \
    -e '/^JobAcctGatherType=/d' \
    "$slurm_conf"

{
    echo "# START AUTO-GENERATED NODE CONFIG"
    cat <<EOF
AccountingStorageType=accounting_storage/slurmdbd
AccountingStorageHost=${hostname}
AccountingStoragePort=6819
JobAcctGatherType=jobacct_gather/cgroup
EOF
    for nodename in $(seq -f "n%02g" 1 ${#nodes_ip[@]}); do
        echo "NodeName=${nodename} CPUs=${cpus_per_node} RealMemory=${real_memory} CoresPerSocket=${cores_per_socket} ThreadsPerCore=${threads_per_core} State=UNKNOWN"
    done
    echo "PartitionName=${partition_name} Nodes=n[01-$(printf "%02g" ${#nodes_ip[@]})] Default=YES MaxTime=INFINITE State=UP"
    echo "# END AUTO-GENERATED NODE CONFIG"
} >> $slurm_conf
# slurmctld refuses to start if clustername has changed,
# remove this file mitigate it
rm -rf /var/spool/slurmctld/clustername
systemctl restart slurmctld slurmdbd
echo "slurmctld is $(systemctl is-active slurmctld)"
echo "slurmdbd is $(systemctl is-active slurmdbd)"

# Give some time for mariadb to start
sleep 3

# Check that sacct works
sacct

# Install Spack
dnf install -y git
test -d /opt/spack/.git || git clone https://github.com/spack/spack.git /opt/spack
chown -R root:root /opt/spack
chmod -R 755 /opt/spack

# Add the Confluent repository
rpm -ivh https://hpc.lenovo.com/yum/latest/el9/x86_64/lenovo-hpc-yum-1-1.x86_64.rpm
dnf install -y lenovo-confluent tftp-server dnsmasq
systemctl enable confluent --now
systemctl enable httpd --now 
systemctl enable tftp.socket --now
systemctl enable dnsmasq --now

# Enable the Confluent environment (or just relogin if you prefer)
set +xu
source /etc/profile.d/confluent_env.sh
set -xu

# Configure SELinux to allow httpd to make connections
setsebool -P httpd_can_network_connect=on

# Configure the internal network interfaces as trusted on FirewallD
if systemctl is-enabled --quiet firewalld.service; then
    firewall-cmd --zone=trusted --change-interface=${internal_iface} --permanent
    firewall-cmd --reload
fi

# Add basic settings to allow a minimalist boot environment for testing
# deployment.useinsecureprotocols=always enable boot over http (instead of https)
nodegroupattrib everything \
    dns.servers=$hn_ip \
    dns.domain=$domain \
    net.ipv4_gateway=$hn_ip \
    deployment.useinsecureprotocols=always \
 
# Fill required passwords
nodegroupattrib everything \
    bmcuser=${bmcuser} \
    bmcpass=${bmcpass} \
    crypted.rootpassword=${rootpass} \
    crypted.grubpassword=${grubpass}

# Generate a keypair for internal cluster usage
test -f ~/.ssh/id_ed25519 || ssh-keygen -f ~/.ssh/id_ed25519 -t ed25519 -N ""

# Define the nodes
for ((i = 0; i < ${#nodes_ip[@]}; i++)); do
    nodename="n$(printf %02d $(($i + 1)))"
    noderemove $nodename 2> /dev/null || :
    nodedefine $nodename 
    nodeattrib $nodename net.hwaddr=${nodes_mac[$i]}
    nodeattrib $nodename net.ipv4_address=${nodes_ip[$i]}/${cidr}
    # deployment.apiarmed=continuous mitigate "Unable to acquire node api key ..." error
    # Note: This is not ideal, make HTTPS boot and api key work in lab environment
    # Note: This deacreses deployment security
    nodeattrib $nodename deployment.apiarmed=continuous
done

# Add nodes to /etc/hosts
# Needs to evaluate in the future if there is a better approach to add files to /etc/hosts; `dnsmasq` maybe?
confluent2hosts -a everything

# Configure the osdeploy parameters
osdeploy initialize -u -s -k -l -p -a -t -g

# Import the OS ISO file.
osdeploy import $iso_path 

# Create a temporary chroot to work as basis for the boot image
export scratchdir=/tmp/scratchdir
rm -rf $scratchdir || :
rm -rf /var/lib/confluent/public/os/${image}-diskless || :
imgutil build -y -s $image $scratchdir

\install -vD -m 0644 -o root  -g root  /etc/hosts                 $scratchdir/etc/hosts
\install -vD -m 0644 -o root  -g root  /etc/passwd                $scratchdir/etc/passwd
\install -vD -m 0644 -o root  -g root  /etc/group                 $scratchdir/etc/group
\install -vD -m 0000 -o root  -g root  /etc/shadow                $scratchdir/etc/shadow
\install -vD -m 0644 -o root  -g root  /etc/security/limits.conf  $scratchdir/etc/security/limits.conf

#
# Customize the image
#

# Install and configure chrony
imgutil exec $scratchdir <<EOF
dnf install -y chrony
sed -e '/^server .* iburst$/d' -i /etc/chrony.conf
echo "server ${hn_ip} iburst" >> /etc/chrony.conf
grep -HE '^server' /etc/chrony.conf
systemctl enable chronyd
EOF

# Install and configure nfs and autofs
imgutil exec $scratchdir <<EOF
dnf install -y autofs
echo "/opt/ohpc/pub /etc/auto.ohpc" > /etc/auto.master.d/ohpc.autofs
echo "* -nfsvers=4 ${hn_ip}:/opt/ohpc/pub/&" > /etc/auto.ohpc

echo "/home /etc/auto.home" > /etc/auto.master.d/home.autofs
echo "* -nfsvers=4 ${hn_ip}:/home/&" > /etc/auto.home
 
echo "/opt/spack /etc/auto.spack" > /etc/auto.master.d/spack.autofs
echo "* -nfsvers=4 ${hn_ip}:/opt/spack/&" > /etc/auto.spack

echo "/opt/intel /etc/auto.intel" > /etc/auto.master.d/intel.autofs
echo "* -nfsvers=4 ${hn_ip}:/opt/intel/&" > /etc/auto.intel
 
echo "/opt/nvidia /etc/auto.nvidia" > /etc/auto.master.d/nvidia.autofs
echo "* -nfsvers=4 ${hn_ip}:/opt/nvidia/&" > /etc/auto.nvidia

# Configure scratch area 
echo "/scratch /etc/auto.scratch" > /etc/auto.master.d/scratch.autofs
echo "local -fstype=xfs,rw :/dev/sda1" > /etc/auto.scratch

# Check that the mounts are correct
grep -H "${hn_ip}" /etc/auto.{home,ohpc,spack,intel,nvidia} 2> /dev/null
 
systemctl enable autofs
EOF

# Slurm node configuration
\install -vD -m 0400 -o munge -g munge /etc/munge/munge.key       $scratchdir/etc/munge/munge.key
\install -vD -m 0644 -o root  -g root  /etc/slurm/slurm.conf      $scratchdir/etc/slurm/slurm.conf
# Install the GPG keys & repos
\cp -va /etc/pki/rpm-gpg/RPM-GPG-KEY-{EPEL-9,OpenHPC-3} $scratchdir/etc/pki/rpm-gpg/
\cp -va /etc/yum.repos.d/{epel,OpenHPC}.repo $scratchdir/etc/yum.repos.d/
imgutil exec $scratchdir <<EOF
set -xeu -o pipefail
dnf install -y ohpc-base-compute ohpc-slurm-client lmod-ohpc hwloc-libs
sed -e '/^account required pam_slurm.so/d' -i /etc/pam.d/sshd
echo 'account required pam_slurm.so' >> /etc/pam.d/sshd
echo SLURMD_OPTIONS=\"--conf-server ${hn_ip}\" > /etc/sysconfig/slurmd
chown munge: /etc/munge/munge.key
systemctl enable munge
systemctl enable slurmd
EOF

if ${ofed_enabled}; then
    # Grabs the folder from the baseurl= line of doca-kernel-$(uname -r).repo
    export doca_repo_folder=$(cat /etc/yum.repos.d/doca-kernel-$(uname -r).repo | perl -lane 'print $1 if /baseurl=file:\/\/(.*)/')
    \cp -va /etc/yum.repos.d/doca.repo $scratchdir/etc/yum.repos.d/
    \cp -va /etc/yum.repos.d/doca-kernel-$(uname -r).repo $scratchdir/etc/yum.repos.d/
    imgutil exec -v $doca_repo_folder:$doca_repo_folder $scratchdir <<EOF
        dnf install --nogpg -y doca-ofed mlnx-ofa_kernel
EOF
fi

# Spack node configuration
imgutil exec $scratchdir <<EOF
cat <<'END' > /etc/profile.d/spack-env.sh
#!/bin/bash

if [ $EUID -eq 0 ] ; then
    # Root gets full Spack functionality
    source /opt/spack/share/spack/setup-env.sh
    source /opt/spack/share/spack/spack-completion.bash
else
    # Normal users get access to prebuilt Spack modules
    export MODULEPATH=/opt/spack/share/spack/lmod/linux-rocky9-x86_64/Core:$MODULEPATH
fi

export LMOD_IGNORE_CACHE=1
END
EOF


# Pack the image
imgutil pack $scratchdir $image-diskless

# Check if the image shows up as available with the defined name
osdeploy list

# Remove the leftover files from the chroot
# rm -rf $scratchdir 

# Configure a given node to be stateless
# -p flag is to ignore BMC commands (if you are running in a system without BMC)
# -n Network boot
for ((i = 0; i < ${#nodes_ip[@]}; i++)); do
    nodename="n$(printf %02d $(($i + 1)))"
    nodedeploy -p $nodename -n $image-diskless
done

echo "Cluster configured, have a nice day 👍"

tail -F /var/log/confluent/{events,audit}
# Reboot n01 and watch the boot process through its monitor (or KVM) and check
# the logs in the headnode
#
# Check /var/log/confluent/trace, if that file exists it means that confluent
# has throwed an exception 

# 
# Testing
# 

# Check the SLURM see the node (it should be idle)
sinfo
# PARTITION AVAIL  TIMELIMIT  NODES  STATE NODELIST
# batchp*      up   infinite      1   idle n01

# Run a test job
srun -N1 -n1 /bin/hostname
# n01

# Check the job accounting
sacct
# JobID           JobName  Partition    Account  AllocCPUS      State ExitCode 
# ------------ ---------- ---------- ---------- ---------- ---------- -------- 
# 1              hostname     batchp       root          1  COMPLETED      0:0 
# 1.0            hostname                  root          1  COMPLETED      0:0 

# Check if node is booted and acessible:
ssh n01

# Check that NFS & autofs is working
df
# Filesystem                               1K-blocks     Used Available Use% Mounted on
# devtmpfs                                   4040996        0   4040996   0% /dev
# tmpfs                                      4061024        0   4061024   0% /dev/shm
# tmpfs                                      4061024     8648   4052376   1% /run
# disklessroot                               3995488   113572   3881916   3% /
# efivarfs                                       256       96       156  39% /sys/firmware/efi/efivars
# 192.168.30.254:/opt/ohpc/pub/libs         82321408 38845440  43475968  48% /opt/ohpc/pub/libs
# tmpfs                                       812204        0    812204   0% /run/user/0
# 192.168.30.254:/opt/ohpc/pub/modulefiles  82321408 38845440  43475968  48% /opt/ohpc/pub/modulefiles
# 192.168.30.254:/opt/spack/share           82321408 38845440  43475968  48% /opt/spack/share
# 192.168.30.254:/opt/spack/bin             82321408 38845440  43475968  48% /opt/spack/bin
# 192.168.30.254:/opt/spack/lib             82321408 38845440  43475968  48% /opt/spack/lib
# 192.168.30.254:/opt/spack/etc             82321408 38845440  43475968  48% /opt/spack/etc


############################

# References:
# <https://hpc.lenovo.com/users/documentation/confluentquickstart_el8.html>
# <https://hpc.lenovo.com/users/documentation/installconfluent_rhel.html>
# <https://hpc.lenovo.com/users/documentation/confluentdiskless.html>
# <https://hpc.lenovo.com/users/documentation/confluentosdeploy.html>
# 
# # Debug Commands
# tcpdump -i ${internal_iface} # Internal network
# tail -F /var/log/confluent/{events,audit,trace} /var/log/httpd/*_log
#
# TODO: 
# - Service Nodes?
# - Storage FS
#   - Lustre?
#   - BeeGFS?
# - GPU
#   References:
#   - https://versatushpc.atlassian.net/wiki/spaces/SV/pages/3933599/Criar+imagem+para+GPU+-+Cuda
#   - https://developer.download.nvidia.com/compute/cuda/repos/rhel9/x86_64/
#   - https://developer.download.nvidia.com/compute/cuda/repos/rhel9/x86_64/cuda-rhel9.repo
#   - Kernel version must be compatible, check at: https://docs.nvidia.com/deploy/cuda-compatibility/
#   Versions to set:
#   - GPU hardware model
#   - CUDA version
#   - Driver version
#   - Kernel version (may require to pin the kernel in the image)
#   - PyTorch / TensorFlow (for AI workloads)
#   - HPC libs: OpenBLAS, cuBLAS, cuFFT need to match CUDA Version
#   Impact:
#   - Slurm configuration requires changes
#   - The image requires changes and possibly pinning the kernel version
#
#   Recommended versions (by GPT5)
#   - **OS / Kernel**
#     - Rocky Linux 9.6
#     - Kernel ~ `5.14.0-570.30.1.el9_6.x86_64`
#   
#   - **NVIDIA Driver**
#     - Recommended: **570.172.08** (works with CUDA 12.8 Update 1)
#     - Other options:
#       - 535.261.03 (with CUDA 12.2 U2)
#       - 580.65.06 (with CUDA 13.0)
#   
#   - **CUDA Toolkit**
#     - CUDA **12.8 Update 1** → driver 570.172.08
#     - CUDA **12.2 Update 2** → driver 535.261.03
#     - CUDA **13.0** → driver 580.65.06
#   
#   - **NCCL / cuDNN / DCGM**
#     - NCCL **2.27.7**
#     - cuDNN **9.12.0**
#     - DCGM ~ **4.4.0**
#   
#   - **Slurm Scheduler**
#     - Use a recent stable release (e.g., **Slurm 23.x LTS**) for GPU scheduling and GRES support.
#   
#   - **UCX / MPI / Communication Layer**
#     - CUDA-aware UCX/MPI matching CUDA version (12.x or 13.0).
#     - Ensure support for CUDA and RDMA/InfiniBand.
#     - Verify MOFED/OFED stack compatibility.
#   
#   - **Miscellaneous**
#     - Kernel headers & dev packages must match the running kernel.
#     - Enable EPEL for extra packages.
#     - Use NVIDIA repos with EL9.6 / Rocky 9.6 support.
#     - Lock versions to prevent automatic update breakage.

