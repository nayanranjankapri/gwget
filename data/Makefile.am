## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

gwget_datadir = $(datadir)/@PACKAGE@

gwget_data_DATA = \
	about.glade\
	newdownload.glade\
	preferences.glade\
	gwget.glade

bonobodir = $(libdir)/bonobo
serverdir = $(libdir)/bonobo/servers
server_in_files = GNOME_Gwget.server.in.in
server_DATA = $(server_in_files:.server.in.in=.server)
$(server_in_files:.server.in.in=.server.in): $(server_in_files)
	sed -e "s|\@BONOBODIR\@|$(bonobodir)|" $< > $@
@INTLTOOL_SERVER_RULE@
                                                                                                             


EXTRA_DIST = $(gwget_data_DATA) \
	     $(server_in_files)
