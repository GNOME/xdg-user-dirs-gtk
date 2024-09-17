project(
  'xdg-user-dirs-gtk', 'c',
        version : '0.10',
        license : 'GPL2+',
  meson_version : '>= 0.50.0'
)

localedir = join_paths (get_option ('prefix'), get_option ('localedir'))
sysconfdir = join_paths (get_option ('prefix'), get_option ('sysconfdir'))

find_program('xdg-user-dirs-update', required: true)

executable ('xdg-user-dirs-gtk-update',
            [ 'parse.c',
              'update.c' ],
            dependencies: dependency('gtk+-3.0'),
            c_args: [ '-DGETTEXT_PACKAGE="xdg-user-dirs-gtk"',
                      '-DGLIBLOCALEDIR="@0@"'.format (localedir),
                      '-DXDG_USER_DIRS_UPDATE=""' ],
            install: true )

i18n = import('i18n')

i18n.merge_file (input: 'user-dirs-update-gtk.desktop.in',
                 output: 'user-dirs-update-gtk.desktop',
                 install: true,
                 install_dir: join_paths (sysconfdir, 'xdg', 'autostart'),
                 po_dir: 'po',
                 type: 'desktop')

subdir('po')
