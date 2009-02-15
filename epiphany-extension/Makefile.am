# The extension needs to installed into Epiphany's prefix,
# not gwget's prefix.  
# ...but that causes problems w/ make distcheck.

extensiondir = $(EPIPHANY_EXTENSIONS_DIR)
extension_LTLIBRARIES = libgwgetextension.la

INCLUDES =				\
 	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"                   \
        -DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"  \
	-I$(top_srcdir)/src		\
	$(DBUS_CFLAGS) \
	-I$(top_builddir)/include \
	@GNOME_CFLAGS@

libgwgetextension_la_SOURCES =			\
 	ephy-gwget.c				\
	ephy-gwget-extension.c			\
	ephy-gwget-extension.h			\
 	$(top_builddir)/src/gwget-application.h \
 	$(top_builddir)/src/gwget-application.c  

libgwgetextension_la_LDFLAGS = -module -avoid-version 

extensionxmldir = $(extensiondir)
extensionxml_in_files = gwget.xml.in.in
extensionxml_DATA = $(extensionxml_in_files:.xml.in.in=.xml)

%.xml.in: %.xml.in.in $(extension_LTLIBRARIES)
	sed -e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
	    -e "s|%LOCALE_DIR%|$(datadir)/locale|" \
	    -e "s|%LIBRARY%|`. ./$(extension_LTLIBRARIES) && echo $$dlname`|" \
	    -e "s|%EXTENSION_DIR%|$(extensiondir)|" \
	$< > $@

@INTLTOOL_XML_RULE@

CLEANFILES = $(extensionxml_DATA)
DISTCLEANFILES = $(extensionxml_DATA)

EXTRA_DIST = $(extensionxml_in_files)
