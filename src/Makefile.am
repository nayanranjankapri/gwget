## Process this file with automake to produce Makefile.in
gladedir = $(datadir)/$(PACKAGE)


INCLUDES = \
	 -DGNOME_GWGET_LOCALEDIR=\""$(datadir)/locale"\" \
	 -DDATADIR=\""$(gladedir)/"\" \
	 -I../include \
	 @GNOME_CFLAGS@

AM_CFLAGS =\
	 -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses -Wpointer-arith -Wmissing-prototypes \
	 -O1\
	 -g

IDL_FILE = GNOME_Gwget.idl

IDL_SOURCES =  \
	GNOME_Gwget-stubs.c             \
	GNOME_Gwget-common.c            \
	GNOME_Gwget-skels.c             \
	GNOME_Gwget.h

BUILT_SOURCES =                         \
        gwget-marshal.c                 \
        gwget-marshal.h                 \
        $(IDL_SOURCES)


$(IDL_SOURCES): $(IDL_FILE) $(ORBIT_IDL)
	$(ORBIT_IDL) -I $(LIBBONOBO_IDL) -I $(BONOBO_ACTIVATION_IDL) $(srcdir)/$(IDL_FILE)

idl_DATA = $(IDL_FILE)
idldir = $(datadir)/idl

EXTRA_DIST = gwget-marshal.list              \
	     $(IDL_FILE)

CLEANFILES = $(BUILT_SOURCES)

bin_PROGRAMS = gwget

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
	eggtrayicon.c\
	systray.c \
	gwget-application-server.c 

gwget-marshal.h: gwget-marshal.list $(GLIB_GENMARSHAL)
	        $(GLIB_GENMARSHAL) $< --header --prefix=gwget_marshal > $@

gwget-marshal.c: gwget-marshal.list $(GLIB_GENMARSHAL)
	echo "#include \"gwget-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=gwget_marshal >> $@
                                                                                                                                                             
gwget_LDADD =  \
	$(GNOME_LIBS)

