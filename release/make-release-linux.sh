#!/bin/bash

#
# Usage example:
# ./make-release-linux.sh -a amd64 -v 2.2 -q /opt/qt/5.12.6/gcc_64/
#

function print_usage () {
    echo Usage:
    echo "    `basename $0` -q <Qt directory> -v <version> -a <architecture>"
}

while getopts ":a:v:q:" name; do
    case $name in
        a)
            ARCH=${OPTARG}
        ;;
        v)
            VERSION=${OPTARG}
        ;;
        q)
            QT_DIR=${OPTARG}
        ;;
        \?)
            echo "Invalid option ${OPTARG}" >&2
            print_usage
            exit 1
        ;;
        :)
            echo "Value of the option ${OPTARG} is required" >&2
            print_usage
            exit 1
        ;;
    esac
done

if [ -z ${ARCH} ]; then
    echo "Architecture is not specified" >&2
    print_usage
    exit 1
fi
if [ -z ${QT_DIR} ]; then
    echo "Qt directory is not specified" >&2
    print_usage
    exit 1
fi
if [ -z ${VERSION} ]; then
    echo "Version is not specified" >&2
    print_usage
    exit 1
fi

echo Using
echo    Architecture: ${ARCH}
echo    Version: ${VERSION}
echo    Qt directory: ${QT_DIR}
echo

EXE_NAME=oplpctools
CURRENT_DIR=`dirname $0`
CURRENT_DIR=`realpath ${CURRENT_DIR}`
ASSETS_DIR="${CURRENT_DIR}/assets"
SRC_DIR=`realpath "${CURRENT_DIR}/.."`
WORKING_DIR="${CURRENT_DIR}/release-linux-${VERSION}-${ARCH}"
BUILD_DIR="${WORKING_DIR}/build"
OUT_DIR="${WORKING_DIR}/${EXE_NAME}"
OUT_BIN_DIR="${OUT_DIR}/bin"
OUT_IMG_DIR="${OUT_DIR}/images"

pushd ${CURRENT_DIR}

if [ -e "$WORKING_DIR" ]; then
    rm -rf "${WORKING_DIR}"
fi
mkdir -pv "${BUILD_DIR}"
mkdir -pv "${OUT_BIN_DIR}"
mkdir -pv "${OUT_IMG_DIR}"

echo Building
cd ${BUILD_DIR}
cmake -DCMAKE_BUILD_TYPE=Release -DQT5_DIR="${QT_DIR}" ${SRC_DIR}
make
chrpath -d ${EXE_NAME}
strip -s --strip-unneeded ${EXE_NAME}


echo Copying application files
cd "${WORKING_DIR}"
cp -v "${BUILD_DIR}/${EXE_NAME}" "${OUT_BIN_DIR}"
cp -v "${BUILD_DIR}"/*.qm "${OUT_BIN_DIR}"
cp -v "${SRC_DIR}/LICENSE.txt" "${OUT_DIR}"
cp -v "${SRC_DIR}/src/OplPcTools/Resources/icons/application.png" "${OUT_IMG_DIR}/icon.png"
cp -v "${ASSETS_DIR}/linux/"* "${OUT_DIR}"
chmod +x "${OUT_DIR}/${EXE_NAME}.sh"

echo Copying Qt translations
for tr in "${OUT_BIN_DIR}"/*.qm; do
    culture=`echo ${tr} | grep -oP "(?<=oplpctools_)[a-z]{2}(?=\.qm)"`
    cp -v "${QT_DIR}/translations/qtbase_${culture}.qm" "${OUT_BIN_DIR}"
done

echo Copying dependencies
mkdir -v "${OUT_BIN_DIR}/platforms"
mkdir -v "${OUT_BIN_DIR}/imageformats"
cp -v "${QT_DIR}/lib/libQt5Core.so.5" "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/lib/libQt5Widgets.so.5" "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/lib/libQt5Network.so.5" "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/lib/libQt5Gui.so.5" "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/lib/libQt5DBus.so.5" "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/lib/libQt5Gui.so.5" "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/lib/libQt5Svg.so.5" "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/lib/libQt5XcbQpa.so.5" "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/lib/libicudata.so."?? "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/lib/libicui18n.so."?? "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/lib/libicuuc.so."?? "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/plugins/platforms/libqxcb.so" "${OUT_BIN_DIR}/platforms/libqxcb.so"
cp -v /usr/lib/x86_64-linux-gnu/libxcb-xkb.so.1 "${OUT_BIN_DIR}"
cp -v /usr/lib/x86_64-linux-gnu/libxcb.so.1 "${OUT_BIN_DIR}"
cp -v /usr/lib/x86_64-linux-gnu/libxkbcommon-x11.so.0 "${OUT_BIN_DIR}"
cp -v /usr/lib/x86_64-linux-gnu/libxkbcommon.so.0 "${OUT_BIN_DIR}"
cp -v "${QT_DIR}/plugins/imageformats/"*.so "${OUT_BIN_DIR}/imageformats"

echo Packing
cd ${WORKING_DIR}
tar czvf "${EXE_NAME}_linux_${VERSION}_${ARCH}.tar.gz" "${EXE_NAME}"

popd
