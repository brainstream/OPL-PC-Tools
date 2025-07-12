#!/bin/bash

#
# Usage example:
# ./make-release-linux.sh -v 3.2
#

set -e

function print_usage () {
    echo Usage:
    echo "    `basename $0` -v <version>"
}

while getopts ":v:" name; do
    case $name in
        v)
            VERSION=${OPTARG}
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

podman build --tag oplpctools --build-arg VERSION=${VERSION} -f Dockerfile .
podman create --name oplpctools -t oplpctools:latest
podman cp --overwrite oplpctools:/oplpctools/oplpctools_linux_${VERSION}_amd64.tar.gz ./release/
podman rm oplpctools
podman image rm -f oplpctools:latest fedora:latest
podman system prune -f
