ARG BASE_IMAGE=debian:bookworm

FROM $BASE_IMAGE AS qt-builder

ARG QT_VERSION=6.9.1

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        g++ \
        cmake \
        ninja-build \
        git \
        python3 \
        build-essential \
        libgl1-mesa-dev \
        libxkbcommon-dev \
        libxcb-xinerama0-dev \
        libfontconfig1-dev \
        libfreetype6-dev \
        libx11-dev \
        libx11-xcb-dev \
        libxcb-randr0-dev \
        libxcb-icccm4-dev \
        libxcb-cursor-dev \
        libxcb-keysyms1-dev \
        libxcb-xinput-dev \
        libxcb-xfixes0-dev \
        libxcb-shape0-dev \
        libxcb-sync-dev \
        libxcb-render-util0-dev \
        libxcb-util-dev \
        libwayland-dev \
        libvulkan-dev \
        libssl-dev \
        libdbus-1-dev \
        libatspi2.0-dev \
        libxcb-image0-dev \
        libxcb-xkb-dev \
        libxkbcommon-x11-dev \
        libfreetype-dev \
        libxcb-glx0-dev \
        libxcb-shm0-dev \
        libxext-dev \
        libxfixes-dev \
        libxi-dev \
        libxrender-dev \
        libxcb1-dev \
        libgtk-3-dev \
        libgtk-4-dev \
        libglib2.0-dev \
        libgdk-pixbuf2.0-dev \
        libpango1.0-dev \
        libcairo2-dev \
        adwaita-icon-theme \
        ca-certificates \
        wget \
        file

WORKDIR /qt

RUN git clone --branch v$QT_VERSION https://code.qt.io/qt/qt5.git && \
    cd qt5 && \
    ./init-repository --module-subset=qtbase,qtsvg,qttools,qttranslations,qtsystems,qtwayland,qtimageformats && \
    mkdir build && \
    cd build && \
    ../configure \
        -prefix /opt/qt6 \
        -release \
        -opensource \
        -confirm-license \
        -nomake examples \
        -nomake tests \
        --doubleconversion=qt \
        -ssl \
        -xcb \
        -glib \
        -gtk \
        -egl \
        --pcre=qt \
        --zlib=qt \
        -gif \
        -ico \
        --libpng=qt \
        --libjpeg=qt \
        --tiff=qt \
        --webp=qt \
        -feature-thread \
        -feature-style-stylesheet \
        -feature-imageformat_bmp \
        -feature-imageformat_jpeg \
        -feature-imageformat_png \
        -feature-imageformat_ppm \
        -feature-imageformat_xbm \
        -feature-imageformat_xpm \
        -feature-imageformatplugin \
        -feature-http \
        -feature-desktopservices \
        --sbom=no \
        -no-dbus \
    && \
    cmake --build . --parallel && \
    cmake --install .

FROM $BASE_IMAGE AS app-builder

ARG VERSION
ARG LOCALES

COPY --from=qt-builder /opt/qt6 /opt/qt6

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        g++ \
        cmake \
        ninja-build \
        patchelf \
        binutils \
        build-essential \
        file \
        wget \
        ca-certificates \
        libgl1-mesa-dev \
        libxkbcommon-dev \
        libxcb-xinerama0-dev \
        libfontconfig1-dev \
        libfreetype6-dev \
        libx11-dev \
        libx11-xcb-dev \
        libxcb-randr0-dev \
        libxcb-icccm4-dev \
        libxcb-cursor-dev \
        libxcb-keysyms1-dev \
        libxcb-xinput-dev \
        libxcb-xfixes0-dev \
        libxcb-shape0-dev \
        libxcb-sync-dev \
        libxcb-render-util0-dev \
        libxcb-util-dev \
        libwayland-dev \
        libvulkan-dev \
        libssl-dev \
        libdbus-1-dev \
        libatspi2.0-dev \
        libxcb-image0-dev \
        libxcb-xkb-dev \
        libxkbcommon-x11-dev \
        libfreetype-dev \
        libxcb-glx0-dev \
        libxcb-shm0-dev \
        libxext-dev \
        libxfixes-dev \
        libxi-dev \
        libxrender-dev \
        libxcb1-dev \
        libgtk-3-dev \
        libgtk-4-dev \
        libglib2.0-dev \
        libgdk-pixbuf2.0-dev \
        libpango1.0-dev \
        libcairo2-dev \
        adwaita-icon-theme && \
    rm -rf /var/lib/apt/lists/*

COPY . /oplpctools/src

WORKDIR /oplpctools/src

RUN mkdir build && \
    cd build && \
    cmake -G Ninja -DQT6_DIR=/opt/qt6 .. && \
    ninja && \
    patchelf --remove-rpath oplpctools && \
    strip -s --strip-unneeded oplpctools


###############################################################################
# tarball
###############################################################################

FROM app-builder AS tarball

WORKDIR /oplpctools/oplpctools

RUN cat > /oplpctools/assemble.sh <<'SCRIPT'
#!/bin/bash
set -e

copy_if_exists() {
    local pattern="$1"
    local dst="$2"
    # Word splitting is intentional here to expand globs
    for f in $pattern; do
        # If glob didn't match, $f is the literal pattern — skip it
        [ -f "$f" ] && cp -Lv "$f" "$dst"
    done
    return 0
}

QT_LIB=/opt/qt6/lib
QT_PLUGINS=/opt/qt6/plugins
TARBALL=/oplpctools/oplpctools

mkdir -p "$TARBALL"/{images,bin/{platforms,imageformats,tls,wayland-decoration-client,wayland-graphics-integration-client,wayland-graphics-integration-server,wayland-shell-integration,egldeviceintegrations,generic,iconengines,platforminputcontexts,platformthemes,xcbglintegrations}}

cp -v /oplpctools/src/LICENSE.txt "$TARBALL/"
cp -v /oplpctools/src/release/assets/linux/oplpctools.sh "$TARBALL/"
cp -v /oplpctools/src/release/assets/linux/make-desktop-file.sh "$TARBALL/"
cp -v /oplpctools/src/src/OplPcTools/Resources/images/application.png "$TARBALL/images/icon.png"

cp -Lv /oplpctools/src/build/oplpctools "$TARBALL/bin/"
cp -Lv /oplpctools/src/build/*.qm "$TARBALL/bin/" 2>/dev/null || true

for lib in \
    libQt6Core.so.6 libQt6Widgets.so.6 libQt6Network.so.6 libQt6Gui.so.6 \
    libQt6Svg.so.6 libQt6XcbQpa.so.6 libQt6OpenGL.so.6 \
    libQt6WaylandEglClientHwIntegration.so.6 libQt6WaylandClient.so.6 \
    libQt6WaylandCompositor.so.6 libQt6WaylandCompositorIviapplication.so.6 \
    libQt6WaylandCompositorPresentationTime.so.6 libQt6WaylandCompositorWLShell.so.6 \
    libQt6WaylandCompositorXdgShell.so.6 libQt6WaylandEglCompositorHwIntegration.so.6 \
    libQt6WlShellIntegration.so.6; do
    cp -Lv "$QT_LIB/$lib" "$TARBALL/bin/"
done

copy_if_exists "$QT_LIB/libicudata.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libicui18n.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libicuuc.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libxcb.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libxcb-xkb.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libdouble-conversion.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libb2.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libpcre2-16.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libxcb-cursor.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libwayland-client.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libwayland-egl.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libxkbcommon.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libxkbcommon-x11.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libssl.so.*" "$TARBALL/bin/"
copy_if_exists "$QT_LIB/libcrypto.so.*" "$TARBALL/bin/"

for search_dir in /lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu; do
    copy_if_exists "$search_dir/libicudata.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libicui18n.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libicuuc.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libssl.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libcrypto.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libxcb.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libxcb-xkb.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libdouble-conversion.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libpcre2-16.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libxcb-cursor.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libwayland-client.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libwayland-egl.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libxkbcommon.so.*" "$TARBALL/bin/"
    copy_if_exists "$search_dir/libxkbcommon-x11.so.*" "$TARBALL/bin/"
done

for plugin_dir in \
    platforms imageformats tls \
    wayland-decoration-client \
    wayland-graphics-integration-client \
    wayland-graphics-integration-server \
    wayland-shell-integration \
    egldeviceintegrations generic iconengines \
    platforminputcontexts platformthemes \
    xcbglintegrations; do
    cp -Lv "$QT_PLUGINS/$plugin_dir/"* "$TARBALL/bin/$plugin_dir/" 2>/dev/null || true
done

QT_TRANSLATIONS=/opt/qt6/translations
for locale in $LOCALES; do
    cp -Lv "$QT_TRANSLATIONS/qt_${locale}.qm" "$TARBALL/bin/" 2>/dev/null || true
    cp -Lv "$QT_TRANSLATIONS/qtbase_${locale}.qm" "$TARBALL/bin/" 2>/dev/null || true
done

chmod +x "$TARBALL/oplpctools.sh"
SCRIPT

RUN bash /oplpctools/assemble.sh

WORKDIR /oplpctools

RUN tar czf oplpctools_linux_${VERSION}_amd64.tar.gz oplpctools


###############################################################################
# AppImage
###############################################################################

FROM app-builder AS appimage

ARG LINUXDEPLOY_VERSION=1-alpha-20251107-1
ARG LINUXDEPLOY_QT_VERSION=1-alpha-20250213-1

RUN wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/${LINUXDEPLOY_VERSION}/linuxdeploy-x86_64.AppImage -O /tmp/linuxdeploy.AppImage && \
    chmod +x /tmp/linuxdeploy.AppImage && \
    cd /usr/local && /tmp/linuxdeploy.AppImage --appimage-extract && \
    mv squashfs-root linuxdeploy && \
    ln -s /usr/local/linuxdeploy/AppRun /usr/local/bin/linuxdeploy && \
    rm /tmp/linuxdeploy.AppImage && \
    wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/${LINUXDEPLOY_QT_VERSION}/linuxdeploy-plugin-qt-x86_64.AppImage -O /tmp/linuxdeploy-plugin-qt.AppImage && \
    chmod +x /tmp/linuxdeploy-plugin-qt.AppImage && \
    cd /usr/local && /tmp/linuxdeploy-plugin-qt.AppImage --appimage-extract && \
    mv squashfs-root linuxdeploy-plugin-qt && \
    ln -s /usr/local/linuxdeploy-plugin-qt/AppRun /usr/local/bin/linuxdeploy-plugin-qt && \
    rm /tmp/linuxdeploy-plugin-qt.AppImage

RUN mkdir -p /oplpctools/AppDir/usr/bin && \
    mkdir -p /oplpctools/AppDir/usr/lib && \
    mkdir -p /oplpctools/AppDir/usr/share/applications && \
    mkdir -p /oplpctools/AppDir/usr/share/icons/hicolor/256x256/apps

RUN cp /oplpctools/src/build/oplpctools /oplpctools/AppDir/usr/bin/oplpctools && \
    cp /oplpctools/src/release/assets/linux/oplpctools.appimage_desktop /oplpctools/AppDir/oplpctools.desktop && \
    cp /oplpctools/src/release/assets/linux/oplpctools.appimage_desktop /oplpctools/AppDir/usr/share/applications/oplpctools.desktop && \
    cp /oplpctools/src/src/OplPcTools/Resources/images/application.png /oplpctools/AppDir/usr/share/icons/hicolor/256x256/apps/oplpctools.png && \
    cp /oplpctools/src/src/OplPcTools/Resources/images/application.png /oplpctools/AppDir/oplpctools.png

RUN cp -v /oplpctools/src/build/*.qm /oplpctools/AppDir/usr/bin/ 2>/dev/null || true
RUN for locale in $LOCALES; do \
        cp -Lv /opt/qt6/translations/qt_${locale}.qm /oplpctools/AppDir/usr/bin/ 2>/dev/null || true; \
        cp -Lv /opt/qt6/translations/qtbase_${locale}.qm /oplpctools/AppDir/usr/bin/ 2>/dev/null || true; \
    done

WORKDIR /oplpctools

RUN ls -la /opt/qt6/bin/qmake && \
    export DEPLOY_QT_VERSION=6 && \
    export QMAKE=/opt/qt6/bin/qmake && \
    export QT_PLUGIN_PATH=/opt/qt6/plugins && \
    export LD_LIBRARY_PATH="/opt/qt6/lib:${LD_LIBRARY_PATH:-}" && \
    export PKG_CONFIG_PATH="/opt/qt6/lib/pkgconfig:${PKG_CONFIG_PATH:-}" && \
    linuxdeploy --appdir AppDir \
        --plugin qt \
    && \
    cp -Lv /opt/qt6/plugins/platforms/libqwayland-generic.so AppDir/usr/plugins/platforms/ && \
    cp -Lv /opt/qt6/plugins/platforms/libqwayland-egl.so AppDir/usr/plugins/platforms/ 2>/dev/null || true && \
    mkdir -p AppDir/usr/plugins/wayland-decoration-client && \
    cp -Lv /opt/qt6/plugins/wayland-decoration-client/* AppDir/usr/plugins/wayland-decoration-client/ 2>/dev/null || true && \
    mkdir -p AppDir/usr/plugins/wayland-shell-integration && \
    cp -Lv /opt/qt6/plugins/wayland-shell-integration/* AppDir/usr/plugins/wayland-shell-integration/ 2>/dev/null || true && \
    mkdir -p AppDir/usr/plugins/wayland-graphics-integration-client && \
    cp -Lv /opt/qt6/plugins/wayland-graphics-integration-client/* AppDir/usr/plugins/wayland-graphics-integration-client/ 2>/dev/null || true && \
    for lib in \
        libQt6WaylandClient.so.6 \
        libQt6WaylandEglClientHwIntegration.so.6 \
        libQt6WaylandCompositor.so.6; do \
        cp -Lv "/opt/qt6/lib/$lib" AppDir/usr/lib/ 2>/dev/null || true; \
    done && \
    for search_dir in /opt/qt6/lib /lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu; do \
        for pattern in \
            "libwayland-client.so.*" \
            "libwayland-egl.so.*" \
            "libwayland-cursor.so.*" \
            "libxkbcommon.so.*"; do \
            for f in "$search_dir"/$pattern; do \
                [ -f "$f" ] && cp -Lv "$f" AppDir/usr/lib/ || true; \
            done; \
        done; \
    done && \
    linuxdeploy --appdir AppDir \
        --output appimage \
        --exclude-library "libssh*" \
        --exclude-library "libnss*" \
        --exclude-library "libldap*" \
        --exclude-library "libqpdf*" \
    && \
    mv *.AppImage oplpctools_${VERSION}_x86_64.AppImage
