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
                                                                                                             
schemasdir = $(sysconfdir)/gconf/schemas
schemas_in_files = gwget.schemas.in
schemas_DATA =  $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@


install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule gwget.schemas

EXTRA_DIST = $(gwget_data_DATA) \
	     $(server_in_files)
