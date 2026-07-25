#!/bin/sh

BINDIR="$(realpath "$(dirname -- "$0")")/bin"

LD_LIBRARY_PATH="$BINDIR" "$BINDIR/oplpctools"
