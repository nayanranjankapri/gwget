/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include "eggtrayicon.h"

enum {
	IMAGE_COLUMN,
	FILENAME_COLUMN,
	URL_COLUMN,
	STATE_COLUMN,
	CURRENTSIZE_COLUMN,
	TOTALSIZE_COLUMN,
	PERCENTAGE_COLUMN,
	TOTALTIME_COLUMN,
	ELAPSETIME_COLUMN,
	CURRENTTIME_COLUMN,
	ESTIMATEDTIME_COLUMN,
	REMAINTIME_COLUMN,
	PID_COLUMN,
	STATE_INT_COLUMN,
	SPEED_COLUMN,
	NUM_COLUMNS
	
};

enum {
        TARGET_URI_LIST,
        TARGET_NETSCAPE_URL,
        TARGET_TEXT_PLAIN
};
                                                                                
/*
 * The used enumeration.
 */
guint dnd_type;


/* target types for dnd */
static  GtkTargetEntry dragtypes[] = {
        { "text/uri-list", 	0, TARGET_URI_LIST },
		{ "x-url/http", 		0, TARGET_NETSCAPE_URL },
        { "x-url/ftp", 		0, TARGET_NETSCAPE_URL },
		{ "_NETSCAPE_URL", 	0, TARGET_NETSCAPE_URL }
};

GladeXML *xml;

/* the model of the GtkTreeView */
/* declared here for be used to add by main_window_cb */
GtkTreeModel *model;

/* GConf Client */
GConfClient *gconf_client;

/* List of all current downloads */
GList *downloads;

/* Tray icon */
EggTrayIcon *tray_icon;
GtkTooltips *tray_tooltip;

/* XML for the preferences gui */
/* It's here because we must load it from main_window.c to put */
/* the options of the column list from Gconf on load */
GladeXML *xml_pref;

GList *donwloads;


void main_window(void);
void on_main_window_delete_event(GtkWidget *widget, gpointer data);
GtkTreeModel* create_model(void);
void add_columns (GtkTreeView *treeview);
void gwget_get_defaults_from_gconf(void);
/* Drag received callback */
void on_treeview_drag_received(GtkWidget * widget, GdkDragContext * context, int x,
                     int y, GtkSelectionData * seldata, guint info,
                     guint time, gpointer data);

void gwget_quit(void);

#endif
