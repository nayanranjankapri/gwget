## Process this file with automake to produce Makefile.in
gladedir = $(datadir)/$(PACKAGE)


INCLUDES = \
	 -DGNOME_GWGET_LOCALEDIR=\""$(datadir)/locale"\" \
	 -DDATADIR=\""$(gladedir)/"\" \
	 -I$(srcdir)/../include \
	 $(NOTIFY_CFLAGS) \
         $(DBUS_CFLAGS) \
	 $(GWGET_CFLAGS) \
	 @GNOME_CFLAGS@

AM_CFLAGS =\
	 -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses -Wpointer-arith -Wmissing-prototypes \
	 -O1\
	 -g

EXTRA_DIST = gwget-application-service.xml 

bin_PROGRAMS = gwget

BUILT_SOURCES =                     

gwget_SOURCES = \
	$(BUILT_SOURCES) \
	main.c\
	main_window.c\
	main_window_cb.c\
	gwget_data.c\
	wget-log.c\
	utils.c\
	custom-cell-renderer-progressbar.c\
	new_window.c\
	systray.c \
	gwget-application.c \
	gwget-application.h 

gwget_LDADD =  \
	$(GWGET_LIBS)  \
	$(NOTIFY_LIBS) \
	$(GNOME_LIBS)

if ENABLE_DBUS
BUILT_SOURCES += gwget-application-service.h gwget-application-client.h
endif

DISTCLEANFILES= \
        gwget-application-service.h \
	gwget-application-client.h

if DBUS_TOOL_NO_PREFIX
gwget-application-service.h: $(srcdir)/gwget-application-service.xml
	dbus-binding-tool --mode=glib-server --output=gwget-application-service.h $(srcdir)/gwget-application-service.xml
gwget-application-client.h: $(srcdir)/gwget-application-service.xml
	dbus-binding-tool --mode=glib-client --output=gwget-application-client.h $(srcdir)/gwget-application-service.xml
	
else
gwget-application-service.h: $(srcdir)/gwget-application-service.xml
	dbus-binding-tool --prefix=gwget_application --mode=glib-server --output=gwget-application-service.h $(srcdir)/gwget-application-service.xml
gwget-application-client.h: $(srcdir)/gwget-application-service.xml
	dbus-binding-tool --prefix=gwget_application --mode=glib-client --output=gwget-application-client.h $(srcdir)/gwget-application-service.xml
endif

