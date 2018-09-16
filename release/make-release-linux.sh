#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Illegal number of parameters"
    exit 1
fi

pushd `pwd`

VERSION=$1
ARCH=$2
EXE_NAME=oplpctools
ASSETS_DIR=`pwd`/assets
SRC_DIR=`pwd`/..
WORKING_DIR=`pwd`/release-linux-${ARCH}
BUILD_DIR=${WORKING_DIR}/build
OUT_DIR=${WORKING_DIR}/${EXE_NAME}
OUT_BIN_DIR=${OUT_DIR}/bin
OUT_IMG_DIR=${OUT_DIR}/images

if [ -e $WORKING_DIR ]; then
    rm -rf $WORKING_DIR
fi
mkdir -p $BUILD_DIR
mkdir -p $OUT_BIN_DIR
mkdir -p $OUT_IMG_DIR

#
# building
#
cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Release $SRC_DIR
make
chrpath -d $EXE_NAME
strip -s --strip-unneeded $EXE_NAME

#
# copying files
#
cd $WORKING_DIR
cp ${BUILD_DIR}/${EXE_NAME} $OUT_BIN_DIR
cp ${BUILD_DIR}/*.qm $OUT_BIN_DIR
cp ${SRC_DIR}/LICENSE.txt $OUT_DIR
cp ${SRC_DIR}/src/OplPcTools/Resources/icons/application.png ${OUT_IMG_DIR}/icon.png
cp ${ASSETS_DIR}/linux/${EXE_NAME}.sh $OUT_DIR
chmod +x $OUT_DIR/${EXE_NAME}.sh

echo
echo IMPORTANT!
echo Copy dependencies to ${OUT_BIN_DIR}
echo

popd
