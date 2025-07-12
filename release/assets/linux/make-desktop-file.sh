#!/bin/sh

FILENAME="oplpctools.desktop"
rm -f "${FILENAME}"

echo "[Desktop Entry]" > "${FILENAME}"
echo "Exec=`pwd`/oplpctools.sh" >> "${FILENAME}"
echo "Icon=`pwd`/images/icon.png" >> "${FILENAME}"
echo "Name=OPL PC Tools" >> "${FILENAME}"
echo "StartupNotify=true" >> "${FILENAME}"
echo "Terminal=false" >> "${FILENAME}"
echo "Type=Application" >> "${FILENAME}"
echo "Categories=Game;Utility;" >> "${FILENAME}"

chmod +x "${FILENAME}"
