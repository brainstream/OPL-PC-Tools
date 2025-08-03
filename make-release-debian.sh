#!/bin/bash

#
# Usage example:
# ./make-release-linux.sh -v 3.2
#

set -e

function print_usage () {
    echo Usage:
    echo "  `basename $0` -v <version> [-k]"
    echo "    -v    OPL PC Tools version"
    echo "    -k    skip docker artifact cleanup"
}

CLEAR_DOCKER=true

while getopts ":v:k" name; do
    case $name in
        v)
            VERSION=${OPTARG}
            echo Building OPL PC Tools v${VERSION}
        ;;
        k)
            CLEAR_DOCKER=false
            echo Docker artifacts will not be cleaned up
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

if [ -z ${VERSION} ]; then
    echo "Version is not specified" >&2
    print_usage
    exit 1
fi

BASE_IMAGE="debian:stable"
QT_VERSION="6.9.1"

if command -v docker &> /dev/null; then
    CRI_CTL=docker
    echo Docker will be used to build image
elif command -v podman &> /dev/null; then
    CRI_CTL=podman
    echo Podman will be used to build image
else
    echo Docker of Podman is required
    exit 1
fi

${CRI_CTL} build \
    --tag oplpctools \
    --build-arg VERSION=${VERSION} \
    --build-arg BASE_IMAGE=${BASE_IMAGE} \
    --build-arg QT_VERSION=${QT_VERSION} \
    -f Dockerfile.debian \
    .
${CRI_CTL} create --name oplpctools -t oplpctools:latest
${CRI_CTL} cp oplpctools:/oplpctools/oplpctools_linux_${VERSION}_amd64.tar.gz ./release/
${CRI_CTL} rm oplpctools

if ${CLEAR_DOCKER}; then
    echo Cleaning up docker artifacts...
    ${CRI_CTL} image rm -f oplpctools:latest ${BASE_IMAGE}
    ${CRI_CTL} system prune
fi
