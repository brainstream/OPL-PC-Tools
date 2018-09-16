#!/bin/sh

BINDIR="`dirname $0`/bin"

export LD_LIBRARY_PATH="$BINDIR"
"$BINDIR/oplpctools"
