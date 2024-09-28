#! /bin/sh

set -eu

mkdir -p "${MESON_INSTALL_DESTDIR_PREFIX}/data/applications/"
cp "${DESTDIR}/etc/xdg/autostart/user-dirs-update-gtk.desktop" \
   "${MESON_INSTALL_DESTDIR_PREFIX}/data/applications/user-dirs-update-gtk.desktop"

