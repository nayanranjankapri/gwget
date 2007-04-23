## Process this file with automake to produce Makefile.in

gwget_datadir = $(datadir)/@PACKAGE@

gwget_data_DATA = \
	preferences.glade\
	gwget.glade

                                                                                                            
# Dbus service file
servicedir = $(datadir)/dbus-1/services
service_in_files = org.gnome.gwget.service.in
service_DATA = $(service_in_files:.service.in=.service)

# Rule to make the service file with bindir expanded
$(service_DATA): $(service_in_files) Makefile
	@sed -e "s|\@bindir\@|$(bindir)|" $< > $@

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = gwget.schemas.in
schemas_DATA =  $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@


install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule gwget.schemas

EXTRA_DIST = $(gwget_data_DATA) \
	     $(service_in_files) \
	     $(schemas_in_files) 
