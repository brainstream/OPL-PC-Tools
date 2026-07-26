#!/bin/bash

#
# Usage example:
# ./make-release-linux.sh -v 4.0
# ./make-release-linux.sh -v 4.0 -t tarball
# ./make-release-linux.sh -v 4.0 -t appimage
#

set -e

function print_usage () {
    echo Usage:
    echo "  $(basename "$0") -v <version> [-t <target>] [-y]"
    echo ""
    echo "  Options:"
    echo "    -v    OPL PC Tools version (required)"
    echo "    -t    Build target: all, tarball, appimage (default: all)"
    echo "    -y    Skip confirmation prompt"
}

TARGET=all
AUTO_YES=false

while getopts ":v:t:y" name; do
    case $name in
        v)
            VERSION=$OPTARG
        ;;
        t)
            TARGET=$OPTARG
            if [[ ! "$TARGET" =~ ^(all|tarball|appimage)$ ]]; then
                echo "Invalid target: $TARGET. Must be: all, tarball, appimage" >&2
                print_usage
                exit 1
            fi
        ;;
        y)
            AUTO_YES=true
        ;;
        \?)
            echo "Invalid option $OPTARG" >&2
            print_usage
            exit 1
        ;;
        :)
            echo "Value of the option $OPTARG is required" >&2
            print_usage
            exit 1
        ;;
    esac
done

if [ -z "$VERSION" ]; then
    echo "Version is not specified" >&2
    print_usage
    exit 1
fi

if command -v docker &> /dev/null; then
    CRI_CTL=docker
elif command -v podman &> /dev/null; then
    CRI_CTL=podman
else
    echo "Docker or Podman is required"
    exit 1
fi

BASE_IMAGE="debian:bookworm"
QT_VERSION="6.9.1"
# White space separated locales
LOCALES="ru"

echo ""
echo "======= Build configuration ======="
echo "  Version:           $VERSION"
echo "  Target:            $TARGET"
echo "  Locales:           $LOCALES"
echo "  Base image:        $BASE_IMAGE"
echo "  Qt version:        $QT_VERSION"
echo "  Container engine:  $CRI_CTL"
echo "==================================="

if ! $AUTO_YES; then
    read -rp "Continue? [Y/n] " answer
    case "$answer" in
        [nN]|[nN][oO])
            echo "Aborted."
            exit 0
        ;;
    esac
fi

BUILD_ARGS=(
    --build-arg "VERSION=$VERSION"
    --build-arg "BASE_IMAGE=$BASE_IMAGE"
    --build-arg "QT_VERSION=$QT_VERSION"
    --build-arg "LOCALES=$LOCALES"
)

mkdir -p ./release

if [[ "$TARGET" == "all" || "$TARGET" == "tarball" ]]; then
    echo ""
    echo "=== Building tarball ==="
    "$CRI_CTL" build \
        "${BUILD_ARGS[@]}" \
        --target tarball \
        --tag oplpctools-tarball \
        -f Dockerfile \
        .
    "$CRI_CTL" create --name oplpctools-tarball oplpctools-tarball:latest
    "$CRI_CTL" cp "oplpctools-tarball:/oplpctools/oplpctools_linux_${VERSION}_amd64.tar.gz" "./release/"
    "$CRI_CTL" rm oplpctools-tarball
    "$CRI_CTL" image rm -f oplpctools-tarball:latest 2>/dev/null || true
    echo "Tarball: ./release/oplpctools_linux_${VERSION}_amd64.tar.gz"
fi

if [[ "$TARGET" == "all" || "$TARGET" == "appimage" ]]; then
    echo ""
    echo "=== Building AppImage ==="
    "$CRI_CTL" build \
        "${BUILD_ARGS[@]}" \
        --target appimage \
        --tag oplpctools-appimage \
        -f Dockerfile \
        .
    "$CRI_CTL" create --name oplpctools-appimage oplpctools-appimage:latest
    "$CRI_CTL" cp "oplpctools-appimage:/oplpctools/oplpctools_${VERSION}_x86_64.AppImage" "./release/"
    "$CRI_CTL" rm oplpctools-appimage
    "$CRI_CTL" image rm -f oplpctools-appimage:latest 2>/dev/null || true
    echo "AppImage: ./release/oplpctools_${VERSION}_x86_64.AppImage"
fi

echo ""
echo "=== Done ==="
echo "Artifacts in ./release/:"
ls -lh "./release/oplpctools_linux_${VERSION}_amd64.tar.gz" "./release/oplpctools_${VERSION}_x86_64.AppImage" 2>/dev/null
