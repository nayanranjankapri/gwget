## Process this file with automake to produce Makefile.in

gwget_datadir = $(datadir)/@PACKAGE@

gwget_data_DATA = \
	preferences.glade\
	gwget.glade

                                                                                                            
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = gwget.schemas.in
schemas_DATA =  $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@


install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule gwget.schemas

EXTRA_DIST = $(gwget_data_DATA) \
	     $(schemas_in_files) 
