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

**YOU MUST HAVE A `/mnt/iso` DIRECTORY IN YOUR HOST WITH ORIGINAL (DEFAULT) ISOs,
SAME DISTRO AND VERSION AS YOUR DESIRED VAGRANT VMs. 
DOWNLOAD IT FROM THE TABLE BELOW**

Run `./setupVagrant /path/to/cloyster_binary 
<vagrant_machine_name1> [<vagrant_machine_name2> ...] ` in this directory.

You can SSH to the machine using `vagrant ssh <vagrant_machine_name>`.

| Name     | Version | ISO                                                                                                                |
|----------|---------|--------------------------------------------------------------------------------------------------------------------|
| alma-8   | 8.9     | [AlmaLinux-8.9-x86_64-dvd.iso](https://repo.almalinux.org/almalinux/8.9/isos/x86_64/AlmaLinux-8.9-x86_64-dvd.iso ) |
| rocky-8  | 8.9     | [Rocky-8.9-x86_64-dvd1.iso](https://download.rockylinux.org/pub/rocky/8/isos/x86_64/Rocky-8.9-x86_64-dvd1.iso)     |
| rocky-9  | 9.3     | [Rocky-9.3-x86_64-dvd.iso](https://download.rockylinux.org/pub/rocky/9/isos/x86_64/Rocky-9.3-x86_64-dvd.iso)       |

