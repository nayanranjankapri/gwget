## Process this file with automake to produce Makefile.in

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
                                                                                                             
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = gwget.schemas.in
schemas_DATA =  $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@


install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule gwget.schemas

EXTRA_DIST = $(gwget_data_DATA) \
	     $(schemas_in_files) \
	     $(server_in_files)
