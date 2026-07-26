#!/bin/sh

FILENAME="oplpctools.desktop"
SCRIPT_DIR="$(realpath "$(dirname -- "$0")")"

cat > "$FILENAME" <<EOF
[Desktop Entry]
Exec="$SCRIPT_DIR/oplpctools.sh"
Icon=$SCRIPT_DIR/images/icon.png
Name=OPL PC Tools
Comment=Graphical PC tools for Open PS2 Loader
StartupNotify=true
Terminal=false
Type=Application
Categories=Game;Utility;
EOF

chmod +x "$FILENAME"
