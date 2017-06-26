#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Illegal number of parameters"
    exit 1
fi

EXEPATH=$1
VERSION=$2
ARCH=$3
OUTDIR=`pwd`

pushd . > /dev/null
cd `dirname $0`
rm -r oplpctools 2>/dev/null
mkdir oplpctools
cp -r debian/* oplpctools
mkdir -p oplpctools/usr/games/
cp ${EXEPATH} oplpctools/usr/games/
strip oplpctools/usr/games/`basename ${EXEPATH}`
mkdir -p oplpctools/usr/share/brainstream/oplpctools
cp ../translations/*.qm oplpctools/usr/share/brainstream/oplpctools/
cp ../src/Resources/icons/application.png oplpctools/usr/share/brainstream/oplpctools/icon.png
sed -i "s/#version#/${VERSION}/g" oplpctools/DEBIAN/control
sed -i "s/#arch#/${ARCH}/g" oplpctools/DEBIAN/control
gzip --best -n oplpctools/usr/share/doc/oplpctools/changelog.Debian
fakeroot dpkg --build oplpctools
mv oplpctools.deb ${OUTDIR}/oplpctools_${VERSION}-1_${ARCH}.deb
lintian ${OUTDIR}/oplpctools_${VERSION}-1_${ARCH}.deb
rm -r oplpctools
popd > /dev/null
