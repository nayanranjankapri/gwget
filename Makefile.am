## Process this file with automake to produce Makefile.in

SUBDIRS = po include src pixmaps data 

if ENABLE_EPIPHANY_EXTENSION
SUBDIRS += epiphany-extension
endif

EXTRA_DIST = \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

desktop_files=gwget.desktop

Applicationsdir = $(datadir)/applications
Applications_DATA = $(desktop_files)

