#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/ansible/roles/slurm.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/runner.h>
#include <cloysterhpc/utils/optional.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace cloyster::services::ansible::roles::slurm {

void run(const Role& role)
{
    using namespace cloyster::utils::singleton;
    namespace optional = cloyster::utils::optional;
    const auto nodesNames = answerfile()->nodes.nodesNames();
    cloyster::functions::abortif(nodesNames.size() < 1,
        "At last one node need to be defined in the answerfile {}",
        answerfile()->path());
    const auto nodesConfig = optional::unwrap(answerfile()->nodes.generic,
        "[node] section not loaded or missing from the answerfile {}",
        answerfile()->path());
    const auto nodesPrefix = optional::unwrap(nodesConfig.prefix,
        "prefix missing in [node] section in the answerfile {}",
        answerfile()->path());
    const auto cpusPerNode = optional::unwrap(nodesConfig.cpus_per_node,
        "cpus_per_node missing in [node] section in the answerfile {}",
        answerfile()->path());
    const auto realMemory = optional::unwrap(nodesConfig.real_memory,
        "real_memory missing in [node] section in the answerfile {}",
        answerfile()->path());
    const auto coresPerSocket = optional::unwrap(nodesConfig.cores_per_socket,
        "cores_per_socket missing in [node] section in the answerfile {}",
        answerfile()->path());
    const auto threadsPerCore = optional::unwrap(nodesConfig.threads_per_core,
        "threads_per_core missing in [node] section in the answerfile {}",
        answerfile()->path());
    const auto sockets = optional::unwrap(nodesConfig.sockets,
        "sockets missing in [node] section in the answerfile {}",
        answerfile()->path());

    runner::shell::fmt(R"del(
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
set +e
mysql -u root --password='' -e "SELECT 1;" >/dev/null 2>&1
if [ $? -eq 0 ]; then
    mysql -u root 2> /dev/null <<EOF
    ALTER USER IF EXISTS 'root'@'localhost' IDENTIFIED BY '{mariadb_root_pass}';

    DELETE FROM mysql.user WHERE User='';
    DELETE FROM mysql.user WHERE User='root' AND Host NOT IN ('localhost', '127.0.0.1', '::1');

    DROP DATABASE IF EXISTS test;
    DELETE FROM mysql.db WHERE Db='test' OR Db='test_%';

    CREATE DATABASE IF NOT EXISTS slurm_acct_db;

    CREATE USER IF NOT EXISTS 'slurm'@'localhost' IDENTIFIED BY '{slurmdb_pass}';
    GRANT ALL PRIVILEGES ON slurm_acct_db.* TO 'slurm'@'localhost';

    FLUSH PRIVILEGES;
EOF
fi
set -e

cat <<EOF > /etc/slurm/slurmdbd.conf
# Authentication
AuthType=auth/munge

# SLURM DB daemon identity
DbdHost={hostname}
DbdPort=6819
SlurmUser=slurm

# Database storage
StorageType=accounting_storage/mysql
StorageHost=localhost
StorageUser=slurm
StoragePass={storage_pass}
StorageLoc=slurm_acct_db

# Logging & PID
LogFile=/var/log/slurmdbd.log
PidFile=/var/run/slurmdbd.pid
EOF
chown slurm:slurm /etc/slurm/slurmdbd.conf
chmod 600 /etc/slurm/slurmdbd.conf

)del",
        fmt::arg("hostname", answerfile()->hostname.hostname),
        fmt::arg(
            "mariadb_root_pass", answerfile()->slurm.mariadb_root_password),
        fmt::arg("slurmdb_pass", answerfile()->slurm.slurmdb_password),
        fmt::arg("storage_pass", answerfile()->slurm.storage_password));

    runner::shell::fmt(R"del(
# Minimal /etc/slurm/slurm.conf
slurm_conf=/etc/slurm/slurm.conf
\cp /etc/slurm/slurm.conf.ohpc $slurm_conf

sed -i \
  -e "s/ClusterName=.*/ClusterName={cluster_name}/" \
  -e "s/SlurmctldHost=.*/SlurmctldHost={hostname}/" \
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

)del",
        fmt::arg("hostname", answerfile()->hostname.hostname),
        fmt::arg("cluster_name", answerfile()->information.cluster_name));

    runner::shell::fmt(R"del(
slurm_conf=/etc/slurm/slurm.conf
{{
    echo "# START AUTO-GENERATED NODE CONFIG"
    cat <<EOF
AccountingStorageType=accounting_storage/slurmdbd
AccountingStorageHost={hostname}
AccountingStoragePort=6819
JobAcctGatherType=jobacct_gather/cgroup
EOF
    for nodename in {node_names}; do
        echo "NodeName=$nodename CPUs={cpus_per_node} Sockets={sockets} RealMemory={real_memory} CoresPerSocket={cores_per_socket} ThreadsPerCore={threads_per_core} State=UNKNOWN"
    done
    echo "PartitionName={partition_name} Nodes={node_prefix}[01-{last_node_num:02d}] Default=YES MaxTime=INFINITE State=UP"
    echo "# END AUTO-GENERATED NODE CONFIG"
}} >> $slurm_conf

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
)del",
        fmt::arg("hostname", answerfile()->hostname.hostname),
        fmt::arg("node_names", fmt::join(nodesNames, " ")),
        fmt::arg("node_prefix", nodesPrefix),
        fmt::arg("cpus_per_node", cpusPerNode), fmt::arg("sockets", sockets),
        fmt::arg("real_memory", realMemory),
        fmt::arg("cores_per_socket", coresPerSocket),
        fmt::arg("threads_per_core", threadsPerCore),
        fmt::arg("last_node_num", nodesNames.size()),
        fmt::arg("partition_name", answerfile()->slurm.partition_name));
}

}
