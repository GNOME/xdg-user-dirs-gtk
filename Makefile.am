NULL =

SUBDIRS=po

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-DLOCALEDIR=\"$(datadir)/locale\" 	\
	-DXDG_USER_DIRS_UPDATE=\"$(XDG_USER_DIRS_UPDATE)\" \
	$(GTK_CFLAGS)				\
	$(NULL)

EXTRA_DIST =					\
	config.rpath				\
        intltool-extract.in                     \
        intltool-merge.in                       \
        intltool-update.in                      \
	user-dirs-update-gtk.desktop		\
	$(NULL)


libraries = $(GTK_LIBS)

bin_PROGRAMS =					\
	xdg-user-dirs-gtk-update		\
	$(NULL)

xdg_user_dirs_gtk_update_SOURCES = parse.c parse.h update.c
xdg_user_dirs_gtk_update_LDADD = $(libraries)

autostartdir = $(sysconfdir)/xdg/autostart
autostart_DATA = user-dirs-update-gtk.desktop

DISTCLEANFILES = \
  intltool-extract \
  intltool-merge \
  intltool-update
