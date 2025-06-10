AC_INIT(elfres.c)
AM_INIT_AUTOMAKE(elfres,0.6.0)
AC_CONFIG_SRCDIR([src/elfres.c])

## Handle target architecture configuration
# Important Note: This step must occur early in the config process
UNAMEM=`uname -m`;
if test "$UNAMEM" = "i386" ; then
	ARCH="i386";
elif test "$UNAMEM" = "i486" ; then
	ARCH="i386";
elif test "$UNAMEM" = "i586" ; then
	ARCH="i386";
elif test "$UNAMEM" = "i686" ; then
	ARCH="i386";
elif test "$UNAMEM" = "x86_64"; then
	ARCH="amd64";
fi
if test "$ARCH" = "i386" ; then
	CFLAGS="$CFLAGS -m32";
	# The LDFLAGS hack below may mess up on actual 32-bit computers,
	# should examine at some point...
	LDFLAGS="$LDFLAGS";
	AC_MSG_NOTICE([Ready for 32-bit compilation.]);
elif test "$ARCH" = "amd64" ; then
	CFLAGS="$CFLAGS -m64";
	LDFLAGS="$LDFLAGS -L/usr/lib";
	AC_MSG_NOTICE([Ready for 64-bit compilation.]);
else
	AC_MSG_ERROR([Target architecture ${ARCH} is invalid!]);
fi
AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(ARCH)
## END Handle target architecture configuration

AC_PROG_CC
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_INSTALL
AM_GNU_GETTEXT([external])

# It is important to make possible both a static and a shared version of
# elfres.  The shared version is for future-proofing, whereas the static
# version should be used to make a more portable binary until libr is
# more widely distributed (as in distributed at all).
AC_ENABLE_SHARED

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# Make sure libr is installed
pkg_modules="libr >= 0.4.0"
PKG_CHECK_MODULES(LIBR, [$pkg_modules])

## Handle GUI configuration
GUI_ENABLED="1"
AC_ARG_ENABLE(gui, [  --disable-gui         disable the elfres GTK+ GUI ], [
	if   test "$enableval" = "yes" ; then
		GUI_ENABLED="1"
		DEBEXTRA=""
	else
		GUI_ENABLED="0"
		DEBEXTRA=""
	fi
])

if   test "$GUI_ENABLED" = "1" ; then
	pkg_modules="gtk+-3.0 >= 3.0.0"
	PKG_CHECK_MODULES(GTK, [$pkg_modules])
	# Check for GtkBuilder support (part of GTK 3)
	AC_DEFINE([USE_GTKBUILDER], [1], [Use GtkBuilder instead of libglade])
fi
AC_SUBST(GUI_ENABLED)
AC_SUBST(DEBEXTRA)
## END Handle GUI configuration

AC_OUTPUT( 
	src/Makefile
	po/Makefile.in
	Makefile
)
