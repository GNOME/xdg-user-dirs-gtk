project(
  'xdg-user-dirs-gtk', 'c',
        version : '0.14',
        license : 'GPL-2.0-or-later',
  meson_version : '>= 0.50.0'
)

localedir = join_paths (get_option ('prefix'), get_option ('localedir'))
sysconfdir = join_paths (get_option ('prefix'), get_option ('sysconfdir'))
datadir = join_paths (get_option ('prefix'), get_option ('datadir'))

find_program('xdg-user-dirs-update', required: true)

executable ('xdg-user-dirs-gtk-update',
            [ 'parse.c',
              'update.c' ],
            dependencies: dependency('gtk+-3.0'),
            c_args: [ '-DGETTEXT_PACKAGE="xdg-user-dirs-gtk"',
                      '-DGLIBLOCALEDIR="@0@"'.format (localedir) ],
            install: true )

i18n = import('i18n')

i18n.merge_file (input: 'user-dirs-update-gtk.desktop.in',
                 output: 'user-dirs-update-gtk.desktop',
                 install: true,
                 install_dir: join_paths (sysconfdir, 'xdg', 'autostart'),
                 po_dir: 'po',
                 type: 'desktop')

# data: Also install desktop file into datadir
# At least on GLib-based systems, autostart folders are not searched
# for installed .desktop files. So in order for GNOME to pick up the
# correct name and icon, install another copy into the datadir's
# application directory.
#
# Mirrors commit: fe37c50488efe8ef9605df239d3e33537cf46332
meson.add_install_script('meson_custom_install_desktop_file.sh', skip_if_destdir: false)

subdir('po')
