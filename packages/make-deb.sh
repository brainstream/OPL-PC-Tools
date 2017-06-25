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
rm -r qpcopl 2>/dev/null
mkdir qpcopl
cp -r debian/* qpcopl
mkdir -p qpcopl/usr/games/
cp ${EXEPATH} qpcopl/usr/games/
strip qpcopl/usr/games/`basename ${EXEPATH}`
mkdir -p qpcopl/usr/share/brainstream/qpcopl
cp ../translations/*.qm qpcopl/usr/share/brainstream/qpcopl/
cp ../src/Resources/icons/application.png qpcopl/usr/share/brainstream/qpcopl/icon.png
sed -i "s/#version#/${VERSION}/g" qpcopl/DEBIAN/control
sed -i "s/#arch#/${ARCH}/g" qpcopl/DEBIAN/control
gzip --best -n qpcopl/usr/share/doc/qpcopl/changelog.Debian
fakeroot dpkg --build qpcopl
mv qpcopl.deb ${OUTDIR}/qpcopl_${VERSION}-1_${ARCH}.deb
lintian ${OUTDIR}/qpcopl_${VERSION}-1_${ARCH}.deb
rm -r qpcopl
popd > /dev/null
