
#include <fmt/core.h>
#include <cloysterhpc/services/confluent.h>
#include <cloysterhpc/services/runner.h>
#include <cloysterhpc/utils/singleton.h>
#include <cloysterhpc/utils/optional.h>
#include <cloysterhpc/functions.h>


namespace cloyster::services {

void Confluent::install() {
    using namespace utils::singleton;
    // NOTE: WIP - GENERALIZE THIS 
    runner::shell::fmt(R"d(
# Add the Confluent repository
rpm -ivh https://hpc.lenovo.com/yum/latest/el{releasever}/{arch}/lenovo-hpc-yum-1-1.{arch}.rpm

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
setsebool -P httpd_can_network_connect=on

# Configure the internal network interfaces as trusted on FirewallD
if systemctl is-enabled -q firewalld; then
    firewall-cmd --zone=trusted --change-interface={internalNic} --permanent
    firewall-cmd --reload
fi

# Add basic settings to allow a minimalist boot environment for testing
nodegroupattrib everything dns.servers={hnIp} dns.domain={domain} net.ipv4_gateway={gateway}

# Fill required passwords
# FIXME: Those strings are get from `stdin` we need to automate also
# nodegroupattrib everything -p bmcuser bmcpass crypted.rootpassword crypted.grubpassword

# Generate a keypair for internal cluster usage
test -f ~/.ssh/id_ed25519 || ssh-keygen -t ed25519 -N ""

# Define a given node
# nodedefine n01
# nodeattrib n01 net.hwaddr=00:0c:29:7d:b6:67
# nodeattrib n01 net.ipv4_address=100.64.0.1/24

# Add nodes to /etc/hosts
# Needs to evaluate in the future if there is a better approach to add files to /etc/hosts; `dnsmasq` maybe?
confluent2hosts -a everything

# Configure the osdeploy parameters; it's an interactive interface, so we must find a way to automate this step
osdeploy initialize -u -s -k -l -p -a -t -g

# Import the OS ISO file.
osdeploy import {isoPath}

# distro contains the osversion
image={distro}-{arch}

rm -rf /tmp/scratchdir || :

# Create a temporary chroot to work as basis for the boot image
imgutil build -y -s ${{image}} /tmp/scratchdir

# Pack the image from the temporary chroot and give a name
imgutil pack /tmp/scratchdir/ ${{image}}-diskless

# Check if the image shows up as available with the defined name
osdeploy list

# Remove the leftover files from the chroot
rm -rf /tmp/scratchdir


)d",

                       fmt::arg("domain", cluster()->getHeadnode().getConnection(Network::Profile::Management).getFQDN()),
                       fmt::arg("releasever", os().getMajorVersion()),
                       fmt::arg("hnIp", cluster()->getHeadnode().getConnection(Network::Profile::Management).getAddress().to_string()),
                       fmt::arg("arch", cloyster::utils::enums::toString(os().getArch())),
                       fmt::arg("distro", os().getDistroString()),
                       fmt::arg("osversion", os().getVersion()),
                       fmt::arg("isoPath", answerfile()->system.disk_image.string()),
                       fmt::arg("internalNic",
                                utils::optional::unwrap(
                                    answerfile()->management.con_interface,
                                    "Internal interface not found in [network_management]"
                                )),
                       fmt::arg("gateway",
                                utils::optional::unwrap(
                                    answerfile()->management.gateway,
                                    "Internal gateway not found in [network_management]"
                               ))
                       );
}

}



