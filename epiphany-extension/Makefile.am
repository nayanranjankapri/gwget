# The extension needs to installed into Epiphany's prefix,
# not gwget's prefix.  
# ...but that causes problems w/ make distcheck.
extensiondir = $(libdir)/epiphany-1.4/extensions

INCLUDES =				\
 	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
 	-DSHARE_DIR=\"pkgdatadir\"	\
 	-DG_DISABLE_DEPRECATED		\
 	-DGDK_DISABLE_DEPRECATED	\
 	-DGTK_DISABLE_DEPRECATED	\
 	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
 	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = libgwgetextension.la

libgwgetextension_la_SOURCES =		\
 	ephy-gwget.c			\
	ephy-gwget-extension.c		\
 	ephy-gwget-extension.h
