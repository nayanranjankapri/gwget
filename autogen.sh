#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="gwget"

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile $PROJECT."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
}

AUTOMAKE=automake-1.7
ACLOCAL=aclocal-1.7

($AUTOMAKE --version) < /dev/null > /dev/null 2>&1 || {
        AUTOMAKE=automake
        ACLOCAL=aclocal
}

($AUTOMAKE --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have automake installed to compile $PROJECT."
	echo "Get ftp://sourceware.cygnus.com/pub/automake/automake-1.4.tar.gz"
	echo "(or a newer version if it is available)"
	DIE=1
}

(test -f $srcdir/configure.in \
  && test -d $srcdir/src \
    && test -f $srcdir/src/gwget_data.c) || {
        echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
	echo " top-level $PKG_NAME directory"
	exit 1
}

gnome_autogen=`which gnome-autogen.sh`
test -z "$gnome_autogen"

USE_GNOME2_MACROS=1 . $gnome_autogen
