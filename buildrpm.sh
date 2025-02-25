#!/bin/bash -eu

VERSION=$(awk '/Version:/ {print $2}' rpmspecs/opencattus.spec)
cp rpmspecs/opencattus.spec ~/rpmbuild/SPECS/
git archive -o ~/rpmbuild/SOURCES/opencattus-${VERSION}.tar.gz \
	--format tgz HEAD --prefix opencattus-${VERSION}/ 
rpmbuild -ba ~/rpmbuild/SPECS/opencattus.spec
echo "RPM Generated"
ls -l ~/rpmbuild/RPMS/*/opencattus-installer*.rpm
