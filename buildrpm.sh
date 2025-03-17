#!/bin/bash -eu

function check_build_distro() {
    local SUPPORTED_BUILD_DISTS=(rocky)
    local ID=$(awk -F= '/^ID=/ { gsub("\"" , "", $2); print $2}' /etc/os-release)
    case "${SUPPORTED_BUILD_DISTS[*]}" in
        "${ID}")
            ;;
        *)
            echo "Build RPM Environment not supported, you need to run this" \
              "in one of these distros: ${SUPPORTED_BUILD_DISTS}"
            exit -1
            ;;
    esac
}

function check_rpmbuild_installed() {
    if ! rpm -q rpmdevtools; then
        sudo dnf -y install rpmdevtools;
        rpmdev-setuptree;
    fi
}

function check_rpmbuild_init() {
    if [ ! -d ~/rpmbuild/SOURCES ]; then
        rpmdev-setuptree
    fi
}

check_build_distro
check_rpmbuild_installed
check_rpmbuild_init

VERSION=$(awk '/Version:/ {print $2}' rpmspecs/opencattus.spec)
cp rpmspecs/opencattus.spec ~/rpmbuild/SPECS/
git archive -o ~/rpmbuild/SOURCES/opencattus-${VERSION}.tar.gz \
    --format tgz HEAD --prefix opencattus-${VERSION}/
rpmbuild -ba ~/rpmbuild/SPECS/opencattus.spec
echo "RPM Generated"
ls -l ~/rpmbuild/RPMS/*/opencattus-installer*.rpm
