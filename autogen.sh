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

(test -f $srcdir/configure.ac \
  && test -d $srcdir/src \
    && test -f $srcdir/src/gwget_data.c) || {
        echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
	echo " top-level $PKG_NAME directory"
	exit 1
}

which gnome-autogen.sh || {
    echo "You need to install gnome-common from the GNOME CVS"
    exit 1
}

REQUIRED_AUTOCONF_VERSION=2.57
REQUIRED_AUTOMAKE_VERSION=1.7
REQUIRED_INTLTOOL_VERSION=0.35.0
REQUIRED_PKG_CONFIG_VERSION=0.16.0

USE_GNOME2_MACROS=1 . gnome-autogen.sh
