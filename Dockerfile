FROM fedora:latest

ARG VERSION

RUN DNF5_FORCE_INTERACTIVE=0 dnf install --setopt=install_weak_deps=false --no-docs --assumeyes \
    g++ cmake ninja patchelf binutils qt6-qtbase-devel qt6-qttools-devel qt6-qtimageformats qt6-qtwayland

COPY . /oplpctools/src

WORKDIR /oplpctools/src

RUN mkdir build && \
    cd build && \
    cmake -G Ninja .. && \
    ninja && \
    patchelf --remove-rpath oplpctools && \
    strip -s --strip-unneeded oplpctools

WORKDIR /oplpctools/oplpctools

RUN mkdir images \
          bin \
          bin/platforms \
          bin/imageformats \
          bin/tls \
          bin/wayland-decoration-client \
          bin/wayland-graphics-integration-client \
          bin/wayland-graphics-integration-server \
          bin/wayland-shell-integration \
          bin/egldeviceintegrations \
          bin/generic \
          bin/iconengines \
          bin/platforminputcontexts \
          bin/platformthemes \
          bin/xcbglintegrations \
        && \
    cp -v \
        /oplpctools/src/LICENSE.txt \
        /oplpctools/src/release/assets/linux/* \
        . && \
    cp -v /oplpctools/src/src/OplPcTools/Resources/images/application.png ./images/icon.png && \
    cp -Lv \
        /oplpctools/src/build/oplpctools \
        /oplpctools/src/build/*.qm \
        /usr/lib64/libQt6Core.so.6 \
        /usr/lib64/libQt6Widgets.so.6 \
        /usr/lib64/libQt6Network.so.6 \
        /usr/lib64/libQt6Gui.so.6 \
        /usr/lib64/libQt6DBus.so.6 \
        /usr/lib64/libQt6Svg.so.6 \
        /usr/lib64/libQt6XcbQpa.so.6 \
        /usr/lib64/libQt6WaylandEglClientHwIntegration.so.6 \
        /usr/lib64/libQt6WaylandClient.so.6 \
        /usr/lib64/libQt6WaylandCompositor.so.6 \
        /usr/lib64/libQt6WaylandCompositorIviapplication.so.6 \
        /usr/lib64/libQt6WaylandCompositorPresentationTime.so.6 \
        /usr/lib64/libQt6WaylandCompositorWLShell.so.6 \
        /usr/lib64/libQt6WaylandCompositorXdgShell.so.6 \
        /usr/lib64/libQt6WaylandEglCompositorHwIntegration.so.6 \
        /usr/lib64/libQt6WlShellIntegration.so.6 \
        /usr/lib64/libQt6WaylandClient.so.6 \
        /usr/lib64/libQt6OpenGL.so.6 \
        /usr/lib64/libicudata.so.?? \
        /usr/lib64/libicui18n.so.?? \
        /usr/lib64/libicuuc.so.?? \
        /usr/lib64/libxcb.so.1 \
        /usr/lib64/libxcb-xkb.so.1 \
        /usr/lib64/libdouble-conversion.so.3 \
        /usr/lib64/libb2.so.1 \
        /usr/lib64/libpcre2-16.so.0 \
        /usr/lib64/libxcb-cursor.so.0 \
        /usr/lib64/libwayland-client.so.0 \
        /usr/lib64/libwayland-egl.so.1 \
        /usr/lib64/libxkbcommon.so.0 \
        /usr/lib64/libxkbcommon-x11.so.0 \
        /usr/lib64/libssl.so.3 \
        /usr/lib64/libcrypto.so.3 \
        bin && \
    cp -Lv /usr/lib64/qt6/plugins/platforms/* bin/platforms && \
    cp -Lv /usr/lib64/qt6/plugins/imageformats/* bin/imageformats && \
    cp -Lv /usr/lib64/qt6/plugins/tls/* bin/tls && \
    cp -Lv /usr/lib64/qt6/plugins/wayland-decoration-client/* bin/wayland-decoration-client && \
    cp -Lv /usr/lib64/qt6/plugins/wayland-graphics-integration-client/* bin/wayland-graphics-integration-client && \
    cp -Lv /usr/lib64/qt6/plugins/wayland-graphics-integration-server/* bin/wayland-graphics-integration-server && \
    cp -Lv /usr/lib64/qt6/plugins/wayland-shell-integration/* bin/wayland-shell-integration && \
    cp -Lv /usr/lib64/qt6/plugins/egldeviceintegrations/* bin/egldeviceintegrations && \
    cp -Lv /usr/lib64/qt6/plugins/generic/* bin/generic && \
    cp -Lv /usr/lib64/qt6/plugins/iconengines/* bin/iconengines && \
    cp -Lv /usr/lib64/qt6/plugins/platforminputcontexts/* bin/platforminputcontexts && \
    cp -Lv /usr/lib64/qt6/plugins/platformthemes/* bin/platformthemes && \
    cp -Lv /usr/lib64/qt6/plugins/xcbglintegrations/* bin/xcbglintegrations && \
    chmod +x oplpctools.sh

WORKDIR /oplpctools

RUN tar czf oplpctools_linux_${VERSION}_amd64.tar.gz oplpctools
