#!/bin/sh

BINDIR="$(realpath "$(dirname -- "$0")")/bin"

LD_LIBRARY_PATH="$BINDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" "$BINDIR/oplpctools"
