top_srcdir = @top_srcdir@
instdir = @bindir@

INCLUDES = \
	-DGUI_ENABLED=@GUI_ENABLED@ \
	@GTK_CFLAGS@ \
	@LIBR_CFLAGS@

bin_PROGRAMS = \
	elfres \
	elficon

# Desktop and icon installation
desktopdir = $(datadir)/applications
desktop_DATA = elfres.desktop

# Install SVG icon to hicolor theme
hicolordir = $(datadir)/icons/hicolor/scalable/apps
hicolor_DATA = elfres.svg

# Create the properly named icon file
elfres.svg: icon.svg
	cp $(srcdir)/icon.svg elfres.svg

# Clean up generated files
CLEANFILES = elfres.svg

# Update icon cache after installation
install-data-hook:
	-gtk-update-icon-cache -f -t $(DESTDIR)$(datadir)/icons/hicolor 2>/dev/null || true

elfres_SOURCES = \
	elfres-gui.c \
	elfres.c

elfres_LDFLAGS = \
	-Wl,--export-dynamic

elfres_LDADD = \
	@GTK_LIBS@ \
	@LIBR_LIBS@

# TODO: Handle symbolic link better (check if exists)
elficon: elfres
	@ln -s ./elfres ./elficon 2> /dev/null || true

# Add the icon, UI file, and "gears image" for our binary
all-local: $(bin_PROGRAMS) $(top_srcdir)/po/de.gmo
	@echo Setting icon for 'elfres'...
	@cp ./elficon ./elficon-tmp
	@cp ./elfres ./elfres-tmp
	@./elficon-tmp -s ./elfres 1d5481ac-052c-11dd-9a75-7751c40e021a
	@./elficon-tmp -a ./elfres "one-canvas.svg" one-canvas.svg
	@./elfres-tmp -a ./elfres ".ui" elfres.ui
	@./elfres-tmp -a ./elfres "gears.svg" gears.svg
	@./elfres-tmp -a ./elfres "gears.png" gears.png
	@echo Adding translations for 'elfres'...
	@./elfres-tmp -a ./elfres "de/LC_MESSAGES/elfres.mo" ${top_srcdir}/po/de.gmo
	@rm ./elficon-tmp ./elfres-tmp
	@echo Post-compile complete.

