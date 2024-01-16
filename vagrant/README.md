# Vagrant

## Configuration

**Please run all commands in this file as root or with elevated privileges.**

### Dependencies

```shell
$ dnf install -y dnf-plugins-core libvirt libvirt-devel qemu-kvm virt-install

$ dnf config-manager --add-repo https://rpm.releases.hashicorp.com/fedora/hashicorp.repo

$ dnf -y install vagrant

$ vagrant plugin install vagrant-libvirt vagrant-scp
```

---

### Configure libvirt

First, enable **libvirt** it by using:
```shell
$ systemctl enable --now libvirtd`

$ systemctl start libvirtd
```

And then you must create a CA cert for it
```shell
$ mkdir -p /etc/pki/CA/private

$ openssl req -x509 -newkey rsa:4096 -keyout /etc/pki/CA/private/cakey.pem -out /etc/pki/CA/cacert.pem -days 3650

$ chmod 600 /etc/pki/CA/private/cakey.pem

$ chmod 644 /etc/pki/CA/cacert.pem
```

---

## Running

Run `./setupVagrant /path/to/cloyster_binary 
<vagrant_machine_name>  <iso_image_path>` in this directory.

You can SSH to the machine using `vagrant ssh <vagrant_machine_name>`.

Already tested ISOs:

| Name       | 8.8     |
|------------|---------|
| RHEL       | Passing |
| RockyLinux | Passing  |
| Alma       | Passing  |
| Oracle     | Passing  |

