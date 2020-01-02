#!/bin/sh

rm -rf result
mkdir result

# rsvg-convert is part of librsvg2-bin
for f in ./*.svg; do
    rsvg-convert -a -w 128 -f svg $f -o result/$f
done
