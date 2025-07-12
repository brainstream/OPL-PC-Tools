#!/bin/sh

BINDIR="$(dirname $0)/bin"

LD_LIBRARY_PATH="$BINDIR" "$BINDIR/oplpctools"
