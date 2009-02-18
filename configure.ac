dnl Process this file with autoconf to produce a configure script.

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(gwget, 1.0.1)
AM_CONFIG_HEADER(config.h)

AC_PATH_PROG(GCONFTOOL, gconftool-2)
IT_PROG_INTLTOOL([0.35.0])

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AM_PROG_LIBTOOL

AM_GCONF_SOURCE_2

LIBGNOMEUI_REQUIRED=2.0.0
LIBGNOME_REQUIRED=2.0.0
GTK_REQUIRED=2.6.0
LIBGLADE_REQUIRED=2.0.0
GCONF_REQUIRED=1.1.11
DBUS_GLIB_REQUIRED=0.33
GIO_REQUIRED=2.16.0

PKG_CHECK_MODULES(GNOME, libgnomeui-2.0 >= $LIBGNOMEUI_REQUIRED
		 	  gtk+-2.0 	>= $GTK_REQUIRED 
			  gmodule-2.0
			  libglade-2.0 	>= $LIBGLADE_REQUIRED
			  gio-unix-2.0 >= $GIO_REQUIRED)
			

AC_SUBST(GNOME_LIBS)
AC_SUBST(GNOME_CFLAGS)
AC_SUBST(GIO_REQUIRED)

dnl ========= Check for DBUS
PKG_CHECK_MODULES([DBUS], [dbus-glib-1 >= $DBUS_GLIB_REQUIRED],
        [enable_dbus=yes],[enable_dbus=no])
AC_SUBST([DBUS_CFLAGS])
AC_SUBST([DBUS_LIBS])

if test "x$enable_dbus" = "xno"; then
        AC_MSG_WARN([DBUS support is disabled since dbus $DBUS_GLIB_REQUIRED or higher was not found])
fi

if test "x$enable_dbus" = "xyes" ; then
        AC_PATH_PROG([DBUS_BINDING_TOOL], [dbus-binding-tool], [no])

        if test x$DBUS_BINDING_TOOL = "xno" ; then
                AC_MSG_ERROR([dbus-binding-tool executable not found in your path - should be installed with dbus glib bindings])
        fi

        AC_DEFINE([ENABLE_DBUS],[1],[Define if DBUS support is enabled])

        DBUS_VERSION=`$PKG_CONFIG --modversion dbus-glib-1 | sed 's/0\.\([[0-9]]*\)\.\{0,1\}\([[0-9]]*\)/\1/'`
        AC_DEFINE_UNQUOTED(DBUS_VERSION, $DBUS_VERSION, [DBUS version.])
fi

GETTEXT_PACKAGE=gwget
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [The gettext package])
ALL_LINGUAS="ar bg ca cs da de dz el en_CA en_GB es eu fi fr hu it ja lt lv mk ne nl oc pa pl pt pt_BR ro ru rw sk sq sv tr uk vi zh_CN zh_HK zh_TW"
AM_GLIB_GNU_GETTEXT

GWGET_CFLAGS="$DBUS_CFLAGS"
GWGET_LIBS="$DBUS_LIBS -lz"

AC_SUBST(GWGET_CFLAGS)
AC_SUBST(GWGET_LIBS)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

if test "x${prefix}" = "xNONE"; then
	AC_DEFINE_UNQUOTED(GNOME_ICONDIR, "${ac_default_prefix}/share/pixmaps", [Gnome pixmap directory])
else
	AC_DEFINE_UNQUOTED(GNOME_ICONDIR, "${prefix}/share/pixmaps", [Gnome pixmap directory])
fi
		                                                                                                                                                                   
if test "x${prefix}" = "xNONE"; then
	AC_DEFINE_UNQUOTED(PIXMAPS_DIR, "${ac_default_prefix}/${DATADIRNAME}/${PACKAGE}/pixmaps", [pixmaps directory])
	pixmapsdir="${ac_default_prefix}/${DATADIRNAME}/${PACKAGE}/pixmaps"
else
	AC_DEFINE_UNQUOTED(PIXMAPS_DIR, "${prefix}/${DATADIRNAME}/${PACKAGE}/pixmaps", [pixmaps directory])
	pixmapsdir="${prefix}/${DATADIRNAME}/${PACKAGE}/pixmaps"
fi
				                                                                                                                                                               
AC_SUBST(pixmapsdir)

AC_ARG_ENABLE([epiphany-extension],
		[  --enable-epiphany-extension   Enable Epiphany Extension ],
		enable_epiphany_extension=$enableval,
		enable_epiphany_extension=yes)



AC_MSG_CHECKING([which epiphany to use])

EPHY_VER=
AC_ARG_WITH([epiphany-version],
        AC_HELP_STRING([--with-epiphany-version@<:@=1.6|1.8|1.10|2.14|2.16|2.18|2.20|2.22|2.24|2.26@:>@],
                       [Whether to use epiphany version 1.6 or 1.8 or 1.10 or 2.14 or 2.16 or 2.18 or 2.20 or 2.22 or 2.24 or 2.26 (default 1.6)]),
        [EPHY_VER="$withval"])

dnl try to autodetect Epiphany 1.2, 1.4, 1.6, 1.8, 1.10, 2.14, 2.15, 2.16, 2.17, 2.18, 2.19, 2.20, 2.21, 2.22, 2.23, 2.24, 2.25 or 2.26
EPHY_AUTODETECT="(autodetect)"
if pkg-config --exists epiphany-2.26; then
        EPHY_VER=2.26
elif pkg-config --exists epiphany-2.25; then
        EPHY_VER=2.25
elif pkg-config --exists epiphany-2.24; then
        EPHY_VER=2.24
elif pkg-config --exists epiphany-2.23; then
        EPHY_VER=2.23
elif pkg-config --exists epiphany-2.22; then
        EPHY_VER=2.22
elif pkg-config --exists epiphany-2.21; then
        EPHY_VER=2.21
elif pkg-config --exists epiphany-2.20; then
        EPHY_VER=2.20
elif pkg-config --exists epiphany-2.19; then
        EPHY_VER=2.19
elif pkg-config --exists epiphany-2.18; then
        EPHY_VER=2.18
elif pkg-config --exists epiphany-2.17; then
        EPHY_VER=2.17
elif pkg-config --exists epiphany-2.16; then
        EPHY_VER=2.16
elif pkg-config --exists epiphany-2.15; then
        EPHY_VER=2.15
elif pkg-config --exists epiphany-2.14; then
        EPHY_VER=2.14
elif pkg-config --exists epiphany-1.10; then
        EPHY_VER=1.10
elif pkg-config --exists epiphany-1.8; then
       	EPHY_VER=1.8
elif pkg-config --exists epiphany-1.6; then
       	EPHY_VER=1.6
elif pkg-config --exists epiphany-1.4; then
	EPHY_VER=1.4
elif pkg-config --exists epiphany-1.2; then
       	EPHY_VER=1.2
fi


EPIPHANY_REQUIRED=
EPIPHANY_PKGCONFIG=
EPIPHANY_SHORTVER=

if test "x$EPHY_VER" = "x1.2"; then
          EPIPHANY_PKGCONFIG=epiphany-1.2
          EPIPHANY_REQUIRED=1.2.0
          EPIPHANY_SHORTVER=12
elif test "x$EPHY_VER" = "x1.4"; then
          EPIPHANY_PKGCONFIG=epiphany-1.4
          EPIPHANY_REQUIRED=1.4.0
          EPIPHANY_SHORTVER=14
elif test "x$EPHY_VER" = "x1.6"; then
          EPIPHANY_PKGCONFIG=epiphany-1.6
          EPIPHANY_REQUIRED=1.6.0
          EPIPHANY_SHORTVER=16
elif test "x$EPHY_VER" = "x1.8"; then
          EPIPHANY_PKGCONFIG=epiphany-1.8
          EPIPHANY_REQUIRED=1.7.0
          EPIPHANY_SHORTVER=18
elif test "x$EPHY_VER" = "x1.10"; then
          EPIPHANY_PKGCONFIG=epiphany-1.10
          EPIPHANY_REQUIRED=1.9.0
          EPIPHANY_SHORTVER=110
elif test "x$EPHY_VER" = "x2.14"; then
          EPIPHANY_PKGCONFIG=epiphany-2.14
          EPIPHANY_REQUIRED=2.14
          EPIPHANY_SHORTVER=214
elif test "x$EPHY_VER" = "x2.15"; then
          EPIPHANY_PKGCONFIG=epiphany-2.15
          EPIPHANY_REQUIRED=2.15
          EPIPHANY_SHORTVER=215
elif test "x$EPHY_VER" = "x2.16"; then
          EPIPHANY_PKGCONFIG=epiphany-2.16
          EPIPHANY_REQUIRED=2.16
          EPIPHANY_SHORTVER=216
elif test "x$EPHY_VER" = "x2.17"; then
          EPIPHANY_PKGCONFIG=epiphany-2.17
          EPIPHANY_REQUIRED=2.17
          EPIPHANY_SHORTVER=217
elif test "x$EPHY_VER" = "x2.18"; then
	  EPIPHANY_PKGCONFIG=epiphany-2.18
          EPIPHANY_REQUIRED=2.18
	  EPIPHANY_SHORTVER=218
elif test "x$EPHY_VER" = "x2.19"; then
	  EPIPHANY_PKGCONFIG=epiphany-2.19
          EPIPHANY_REQUIRED=2.19
	  EPIPHANY_SHORTVER=219
elif test "x$EPHY_VER" = "x2.20"; then
	  EPIPHANY_PKGCONFIG=epiphany-2.20
          EPIPHANY_REQUIRED=2.20
	  EPIPHANY_SHORTVER=220
elif test "x$EPHY_VER" = "x2.21"; then
	  EPIPHANY_PKGCONFIG=epiphany-2.21
          EPIPHANY_REQUIRED=2.21
	  EPIPHANY_SHORTVER=221
elif test "x$EPHY_VER" = "x2.22"; then
	  EPIPHANY_PKGCONFIG=epiphany-2.22
          EPIPHANY_REQUIRED=2.22
	  EPIPHANY_SHORTVER=222
elif test "x$EPHY_VER" = "x2.23"; then
	  EPIPHANY_PKGCONFIG=epiphany-2.23
          EPIPHANY_REQUIRED=2.23
	  EPIPHANY_SHORTVER=223
elif test "x$EPHY_VER" = "x2.24"; then
	  EPIPHANY_PKGCONFIG=epiphany-2.24
          EPIPHANY_REQUIRED=2.24
	  EPIPHANY_SHORTVER=224
elif test "x$EPHY_VER" = "x2.25"; then
	  EPIPHANY_PKGCONFIG=epiphany-2.25
          EPIPHANY_REQUIRED=2.25
	  EPIPHANY_SHORTVER=225
elif test "x$EPHY_VER" = "x2.26"; then
	  EPIPHANY_PKGCONFIG=epiphany-2.26
          EPIPHANY_REQUIRED=2.26
	  EPIPHANY_SHORTVER=226
else
     	AC_MSG_RESULT([no epiphany installed])
fi

if test -n "$EPIPHANY_SHORTVER" ; then
AC_DEFINE_UNQUOTED(EPHY_VERSION, "$EPIPHANY_SHORTVER",
       [Define to the Epiphany version (12, 14, 16, 18, 110, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225 or 226)])
AC_MSG_RESULT([$EPHY_VER $EPHY_AUTODETECT])

LIBGLIB_REQUIRED=2.4.0
LIBGTK_REQUIRED=2.4.0

AC_SUBST(EPIPHANY_REQUIRED)
AC_SUBST(LIBGLIB_REQUIRED)
AC_SUBST(LIBGTK_REQUIRED)

PKG_CHECK_MODULES(EPIPHANY_DEPENDENCY, \
                  glib-2.0 >= $LIBGLIB_REQUIRED \
                  gmodule-2.0 \
                  gtk+-2.0 >= $LIBGTK_REQUIRED \
                  $EPIPHANY_PKGCONFIG >= $EPIPHANY_REQUIRED \
		  )
AC_SUBST(EPIPHANY_DEPENDENCY_CFLAGS)
AC_SUBST(EPIPHANY_DEPENDENCY_LIBS)

if test "x$EPHY_VER" = "x1.2"; then
EPIPHANY_EXTENSIONS_DIR="`$PKG_CONFIG --variable=libdir epiphany-1.2`/epiphany/extensions"
else
EPIPHANY_EXTENSIONS_DIR="`$PKG_CONFIG --variable=extensionsdir epiphany-$EPHY_VER`"
fi
AC_SUBST([EPIPHANY_EXTENSIONS_DIR])

else
	enable_epiphany_extension=no
fi
AM_CONDITIONAL(ENABLE_EPIPHANY_EXTENSION, test "x$enable_epiphany_extension" = "xyes")


#
# Check for libnotify
#
LIBNOTIFY_REQUIRED=0.2.2
AC_ARG_ENABLE(libnotify,
            AC_HELP_STRING([--disable-libnotify],
                           [Disable libnotify support]),,
              enable_libnotify=auto)
if test "x$enable_libnotify" != "xno"; then
        PKG_CHECK_MODULES(NOTIFY,                            \
                          libnotify >= $LIBNOTIFY_REQUIRED,
                          have_libnotify=yes,
                          have_libnotify=no)
        if test "x$have_libnotify" = "xno" -a "x$enable_libnotify" = "xyes"; then
          AC_MSG_ERROR([libnotify support explicitly requested, but libnotify couldn't be found])
        fi
        if test "x$have_libnotify" = "xyes"; then
             enable_libnotify=yes
        fi
fi
AM_CONDITIONAL(USE_NOTIFY, test x"$enable_libnotify" = xyes)
if test x$enable_libnotify = xyes ; then
    # Find out the version of LIBNOTIFY we're using
    libnotify_version=`pkg-config --modversion libnotify`
    LIBNOTIFY_VERSION_MAJOR=`echo $libnotify_version | awk -F. '{print $1}'`
    LIBNOTIFY_VERSION_MINOR=`echo $libnotify_version | awk -F. '{print $2}'`
    LIBNOTIFY_VERSION_MICRO=`echo $libnotify_version | awk -F. '{print $3}'`
    if test "z$LIBNOTIFY_VERSION_MAJOR" = "z"; then
        LIBNOTIFY_VERSION_MAJOR="0"
    fi
    if test "z$LIBNOTIFY_VERSION_MINOR" = "z"; then
        LIBNOTIFY_VERSION_MINOR="0"
    fi
    if test "z$LIBNOTIFY_VERSION_MICRO" = "z"; then
        LIBNOTIFY_VERSION_MICRO="0"
    fi
    echo "Your libnotify version is $LIBNOTIFY_VERSION_MAJOR,$LIBNOTIFY_VERSION_MINOR,$LIBNOTIFY_VERSION_MICRO."
    NOTIFY_CFLAGS="$NOTIFY_CFLAGS -DLIBNOTIFY_VERSION_MAJOR=$LIBNOTIFY_VERSION_MAJOR"
    NOTIFY_CFLAGS="$NOTIFY_CFLAGS -DLIBNOTIFY_VERSION_MINOR=$LIBNOTIFY_VERSION_MINOR"
    NOTIFY_CFLAGS="$NOTIFY_CFLAGS -DLIBNOTIFY_VERSION_MICRO=$LIBNOTIFY_VERSION_MICRO"

    AC_DEFINE(HAVE_NOTIFY, 1, [Define if libnotify support is enabled])
    AC_SUBST(NOTIFY_CFLAGS)
    AC_SUBST(NOTIFY_LIBS)
fi



AM_CONDITIONAL([ENABLE_DBUS], [test "x$enable_dbus" = "xyes"])
AM_CONDITIONAL([DBUS_TOOL_NO_PREFIX], [test "x$DBUS_VERSION" = "x33"])

AC_ARG_WITH([extension-prefix],
    [  --with-extension-prefix=path              Epiphany extension prefix],    
    [extension_prefix=$withval],
    [extension_prefix=/]
)
EXTENSION_PREFIX="$extension_prefix"
AC_SUBST([EXTENSION_PREFIX])

dnl Stuff for the CORBA interface
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

ORBIT_IDL="`$PKG_CONFIG --variable=orbit_idl ORBit-2.0`"
AC_SUBST(ORBIT_IDL)
                                                                                                                                                              
AC_OUTPUT([
Makefile
po/Makefile.in
src/Makefile
pixmaps/Makefile
data/Makefile
epiphany-extension/Makefile
])
