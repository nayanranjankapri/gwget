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
 
#include <config.h>
#include <glade/glade.h>
#include <gnome.h>
#include <gconf/gconf-client.h>
#include <glib/gstdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "main_window.h"
#include "main_window_cb.h"
#include "gwget_data.h"
#include "custom-cell-renderer-progressbar.h"
#include "utils.h"
#include "systray.h"
#include "new_window.h"

/* Global Preferences */
Preferences gwget_pref;

static void 
show_prefered_columns(void);

static gboolean
gwget_destination_file_exists(GwgetData *data);

static void 
gwget_gconf_notify_toolbar(GConfClient *client,
			   guint        cnxn_id,
		   	   GConfEntry  *entry,
		   	   gpointer     user_data);

void 
main_window(void) 
{
	GtkWidget * window = NULL;
	gchar *xml_file = NULL,*toolbar_setting;
	GtkWidget *treev,*toolbar,*menu_item, *combo;
	GtkTreeSelection *select;
	

	if (!xml) {
		xml_file=g_build_filename(DATADIR,"gwget.glade",NULL);
		xml = glade_xml_new(xml_file,NULL,NULL);
	
		glade_xml_signal_autoconnect(xml);
	}
	
	if (!xml_new) {
		xml_file =g_build_filename(DATADIR, "newdownload.glade", NULL);
		xml_new = glade_xml_new(xml_file, NULL, NULL);
		glade_xml_signal_autoconnect (xml_new);
	}
	
	
	window = glade_xml_get_widget(xml,"main_window");
	treev = glade_xml_get_widget(xml,"treeview1");
	model = create_model();
	gtk_tree_view_set_model(GTK_TREE_VIEW(treev),GTK_TREE_MODEL(model));
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treev));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);

	/*
	 * Update window size in gconf on resize 
	 */
	g_signal_connect(GTK_WIDGET(window),
			 "configure-event",
			 G_CALLBACK(gwget_remember_window_size_and_position),
			 NULL);
	
	/* add the columns titles to the tree view */
	add_columns (GTK_TREE_VIEW (treev));
	
	gconf_client = gconf_client_get_default();
	gconf_client_add_dir (gconf_client, 
			      "/apps/gwget2", 
			      GCONF_CLIENT_PRELOAD_NONE,
                              NULL);
	
	/* gwget_pref.download_dir=NULL; */
	gwget_get_defaults_from_gconf();
	
	/* Drag and drop set up */
	gtk_drag_dest_set(GTK_WIDGET(window), 
			  GTK_DEST_DEFAULT_ALL | GTK_DEST_DEFAULT_HIGHLIGHT,
			  dragtypes, 
			  sizeof(dragtypes) / sizeof(dragtypes[0]),
                          GDK_ACTION_COPY);
						
	g_signal_connect(G_OBJECT(window), 
			 "drag_data_received",
			 G_CALLBACK(on_gwget_drag_received),
			 GUINT_TO_POINTER(dnd_type));	
					 
	/* Set the toolbar like gnome preferences */
	toolbar = glade_xml_get_widget(xml,"toolbar1");
	toolbar_setting = gconf_client_get_string(gconf_client,"/desktop/gnome/interface/toolbar_style",NULL);
	
	if (!strcmp(toolbar_setting,"icons")) {
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_ICONS);
	}
	
	if (!strcmp(toolbar_setting,"both")) {
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_BOTH);
	}
	
	if (!strcmp(toolbar_setting,"both-horiz")) {
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_BOTH_HORIZ);
	}
	
	if (!strcmp(toolbar_setting,"text")) {
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_TEXT);
	}
	
	/* Listen to changes to the key. */
	gconf_client_notify_add (gconf_client,
				"/desktop/gnome/interface/toolbar_style",
				gwget_gconf_notify_toolbar,
				NULL,
				NULL,
				NULL);
	
	/* Show the toolbar ? */
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(xml,"view_toolbar")),gwget_pref.view_toolbar);
	toolbar = glade_xml_get_widget(xml,"bonobotoolbar"); 
	menu_item=glade_xml_get_widget(GLADE_XML(xml),"view_toolbar");
	if (gwget_pref.trayonly) {
		gtk_widget_show(GTK_WIDGET(window));
		gtk_widget_hide(GTK_WIDGET(window));
	}
	else 
		gtk_widget_show(GTK_WIDGET(window));

	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item))) {
		gtk_widget_hide(GTK_WIDGET(toolbar));
	} else {
		gtk_widget_show(GTK_WIDGET(toolbar));
	}

	
	systray_load();

	/* Create the model for the "save in" option in new download dialog */
	save_in_model = (GtkTreeModel*)gtk_list_store_new (1, G_TYPE_STRING);
	combo = glade_xml_get_widget (xml_new, "save_in_comboboxentry");
	gtk_combo_box_set_model(GTK_COMBO_BOX(combo), save_in_model);
	gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY(combo), 0);
	gtk_entry_set_text(GTK_ENTRY(GTK_BIN(combo)->child), gwget_pref.download_dir);
		
}

static gboolean
gwget_destination_file_exists(GwgetData *data)
{
	struct stat s;
	
	return ( g_stat(data->local_filename,&s) == 0 );
}

void 
gwget_get_defaults_from_gconf(void)
{
	gint num_dl, i, total_size;
	GwgetData *data;
	gchar *key,*url,*dir,*name;
	DlState state;
	gint default_width, default_height;
	GError *error = NULL;
	
	gwget_pref.http_proxy=gconf_client_get_string(gconf_client,"/apps/gwget2/http_proxy",NULL);
	gwget_pref.http_proxy_port=gconf_client_get_int(gconf_client,"/apps/gwget2/http_proxy_port",NULL);
	gwget_pref.gnome_http_proxy=gconf_client_get_string(gconf_client,"/system/http_proxy/host",NULL);
	gwget_pref.gnome_http_proxy_port=gconf_client_get_int(gconf_client,"/system/http_proxy/port",NULL);
	gwget_pref.gnome_use_proxy=gconf_client_get_bool(gconf_client,"/system/http_proxy/use_http_proxy",NULL);
	gwget_pref.network_mode=gconf_client_get_string(gconf_client,"/apps/gwget2/network_mode",NULL);
	gwget_pref.download_dir=gconf_client_get_string(gconf_client,"/apps/gwget2/download_dir",NULL);
	gwget_pref.num_retries=gconf_client_get_int(gconf_client,"/apps/gwget2/num_retries",NULL);
	gwget_pref.resume_at_start=gconf_client_get_bool(gconf_client,"/apps/gwget2/resume_at_start",NULL);
	gwget_pref.open_after_dl=gconf_client_get_bool(gconf_client,"/apps/gwget2/open_after_dl",NULL);
	gwget_pref.no_create_directories=gconf_client_get_bool(gconf_client,"/apps/gwget2/no_create_directories",NULL);
	gwget_pref.follow_relative=gconf_client_get_bool(gconf_client,"/apps/gwget2/follow_relative",NULL);
	gwget_pref.convert_links = gconf_client_get_bool(gconf_client,"/apps/gwget2/convert_links",NULL);	
	gwget_pref.dl_page_requisites = gconf_client_get_bool(gconf_client,"/apps/gwget2/dl_page_requisites",NULL);
	gwget_pref.max_depth=gconf_client_get_int(gconf_client,"/apps/gwget2/max_depth",NULL);
	gwget_pref.view_actual_size=gconf_client_get_bool(gconf_client,"/apps/gwget2/view_actual_size",NULL);
	gwget_pref.view_total_size=gconf_client_get_bool(gconf_client,"/apps/gwget2/view_total_size",NULL);
	gwget_pref.view_percentage=gconf_client_get_bool(gconf_client,"/apps/gwget2/view_percentage",NULL);
	gwget_pref.view_elapse_time=gconf_client_get_bool(gconf_client,"/apps/gwget2/view_elapse_time",NULL);
	gwget_pref.view_down_speed=gconf_client_get_bool(gconf_client,"/apps/gwget2/view_down_speed",NULL);
	gwget_pref.view_toolbar=gconf_client_get_bool(gconf_client,"/apps/gwget2/view_toolbar",NULL);
	gwget_pref.view_file_type=gconf_client_get_bool(gconf_client,"/apps/gwget2/view_file_type",NULL);
	gwget_pref.limit_speed = gconf_client_get_bool (gconf_client,"/apps/gwget2/limit_speed", NULL);
	gwget_pref.max_speed=gconf_client_get_int(gconf_client,"/apps/gwget2/max_speed",NULL);
	gwget_pref.limit_simultaneousdownloads = gconf_client_get_bool (gconf_client,"/apps/gwget2/limit_simultaneousdownloads", NULL);
	gwget_pref.max_simultaneousdownloads=gconf_client_get_int(gconf_client,"/apps/gwget2/max_simultaneousdownloads",NULL);
	gwget_pref.ask_save_each_dl = gconf_client_get_bool (gconf_client,"/apps/gwget2/ask_save_each_dl", NULL);
	
	if (!gwget_pref.download_dir) {
		gwget_pref.download_dir = g_strdup(g_get_home_dir());
		gconf_client_set_string (gconf_client, "/apps/gwget2/download_dir", g_strdup(g_get_home_dir()), NULL);
	}

	if (!gwget_pref.network_mode) {
		gwget_pref.network_mode = g_strdup("direct");
		gconf_client_set_string (gconf_client, "/apps/gwget2/network_mode", "direct", NULL);
	}

	/* Put in the list of save_in_paths (used by new dialog window) the initial download directory */
	save_in_paths = g_list_append (save_in_paths, gwget_pref.download_dir);
		
	num_dl=gconf_client_get_int(gconf_client,"/apps/gwget2/n_downloads",NULL);
	for (i=0;i<num_dl;i++) {
		key=g_strdup_printf("/apps/gwget2/downloads_data/%d/url",i);
		url=gconf_client_get_string(gconf_client,key,NULL);
		key=g_strdup_printf("/apps/gwget2/downloads_data/%d/dir",i);
		dir=gconf_client_get_string(gconf_client,key,NULL);
		
		data=gwget_data_create(url,dir);
		
		key=g_strdup_printf("/apps/gwget2/downloads_data/%d/filename",i);
		name=gconf_client_get_string(gconf_client,key,NULL);
		gwget_data_set_filename(data,name);
		
		key=g_strdup_printf("/apps/gwget2/downloads_data/%d/state",i);
		state=gconf_client_get_int(gconf_client,key,NULL); 
		
		key=g_strdup_printf("/apps/gwget2/downloads_data/%d/total_size",i);
		total_size = gconf_client_get_int (gconf_client, key, &error);
		if (!error) {		
			gwget_data_set_total_size (data, total_size);
		} else { 
			gwget_data_set_total_size (data, 0);
		}

		if ( state != DL_COMPLETED ) {
			/*
			 * If the download is not completed - add it , no questions
			 */	
			new_download(data);
		} else {
			/* 
			 * If the download is completed , then:
			 * if the file we want to write to is missing , the user
			 * has (re)moved it , so quietly forget download , otherwise
			 * add it
		 	 */
			if ( !gwget_destination_file_exists(data) ) {
				/*
				 * We do not add download, the gwget_remember_downloads
				 * call after the cycle will flush gconf
				 */
				continue;
			} else {
				new_download(data);
				gwget_data_set_state(data,DL_COMPLETED);
				continue;
			}
		}
		    		
		gwget_data_set_state(data,DL_NOT_RUNNING); 
		if (gwget_pref.resume_at_start && data->state!=DL_COMPLETED) {
			gwget_data_start_download(data);
		} 
		/* FIXME: put all the cases (error, retriving...) */
		
		if (state==DL_COMPLETED) {
			gwget_data_set_state(data, state);
		}
	}
	/*
	 * We may have altered the download list so we sync gconf
	 */
	gwget_remember_downloads();
	
	show_prefered_columns();

	/* Default width and height */
	default_width=gconf_client_get_int (gconf_client,"/apps/gwget2/default_width",NULL);
	default_height=gconf_client_get_int (gconf_client,"/apps/gwget2/default_height",NULL);
	gtk_window_resize (GTK_WINDOW (glade_xml_get_widget(xml,"main_window")),default_width,default_height);
	
	/* Default position */
	gtk_window_move(GTK_WINDOW(glade_xml_get_widget(xml,"main_window")),
					gconf_client_get_int (gconf_client,"/apps/gwget2/position_x",NULL),
					gconf_client_get_int (gconf_client,"/apps/gwget2/position_y",NULL)
					);
}			


void 
on_main_window_delete_event(GtkWidget *widget, gpointer data)
{
	if (gwget_pref.docked) {
		gtk_widget_hide(widget);
	} else {
		gwget_quit();
	}
}

GtkTreeModel *
create_model (void)
{
	GtkListStore *model;

	model = gtk_list_store_new (NUM_COLUMNS,
				    GDK_TYPE_PIXBUF,
				    G_TYPE_STRING, /* File name */
			 	    G_TYPE_STRING, /* URL */
				    G_TYPE_STRING, /* State */
				    G_TYPE_STRING, /* Current size */
				    G_TYPE_STRING, /* Total Size */
				    G_TYPE_INT,    /* Percentage */
				    G_TYPE_STRING, /* Percentage */
				    G_TYPE_STRING, /* Elapse Time */
				    G_TYPE_STRING, /* Current time  */
				    G_TYPE_STRING, /* Estimated time  */
				    G_TYPE_STRING, /* Remain Time */
				    /* Not viewable columns */ 
				    G_TYPE_INT,    /* Pid */
				    G_TYPE_INT,	   /* State int column */
				    G_TYPE_STRING  /* Speed */
				);
				  
	return GTK_TREE_MODEL (model);
}

void
add_columns (GtkTreeView *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	
	/* File Name Column */
	
	renderer = gtk_cell_renderer_pixbuf_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start (column,
					renderer,
					FALSE);
	gtk_tree_view_column_add_attribute(column,
					  renderer,
					  "pixbuf", 
					  IMAGE_COLUMN);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column,
					renderer,
					FALSE);
	gtk_tree_view_column_add_attribute (column,
					  renderer, 
					  "text",
					  FILENAME_COLUMN);
	
	gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN(column),_("File Name"));
	
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_append_column (treeview, column);
	
	/* State Column */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("State"),
							  renderer,
							  "text",
							  STATE_COLUMN,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column (treeview, column);
	
	/* Current Size */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Current Size"),
							renderer,
						     	"text",
							CURRENTSIZE_COLUMN,
							NULL);

	gtk_tree_view_column_set_sort_column_id (column, CURRENTSIZE_COLUMN);
  	gtk_tree_view_append_column (treeview, column);
	
	/* Total Size */
	renderer = gtk_cell_renderer_text_new ();
    	column = gtk_tree_view_column_new_with_attributes (_("Total Size"),
							renderer,
							"text",
							TOTALSIZE_COLUMN,
							NULL);
	gtk_tree_view_column_set_sort_column_id (column, TOTALSIZE_COLUMN);
  	gtk_tree_view_append_column (treeview, column);
	
	/* Percentage */
	renderer = ephy_cell_renderer_progress_new();
	column = gtk_tree_view_column_new_with_attributes (_("Percentage"),
							renderer,
							"value", 
							PERCENTAGE_COLUMN,
							NULL);

	gtk_tree_view_column_set_title (column, (_("Percentage"))); 
    	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),column); 
		
	/* Elapse Time */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Elapsed Time"),
							renderer,
							"text",
							ELAPSETIME_COLUMN,
							NULL);
	gtk_tree_view_column_set_sort_column_id (column, ELAPSETIME_COLUMN);
	gtk_tree_view_append_column (treeview, column);
	
	/* REMAINTIME_COLUMN */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Remaining Time"),
														renderer,
														"text",
														REMAINTIME_COLUMN,
														NULL);
	gtk_tree_view_column_set_sort_column_id (column, REMAINTIME_COLUMN);
	gtk_tree_view_append_column (treeview, column);
	
	/* Speed */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Speed"),
														renderer,
														"text",
														SPEED_COLUMN,
														NULL);
	gtk_tree_view_column_set_sort_column_id (column, SPEED_COLUMN);
	gtk_tree_view_append_column (treeview, column);
	
}

void 
on_gwget_drag_received (GtkWidget * widget, GdkDragContext * context, int x,
							int y, GtkSelectionData * seldata, guint info,	
							guint time, gpointer data)
{
	gchar *file;
	GList *files;
	GwgetData *gwgetdata;
	gboolean drop_ok;
	
	g_return_if_fail(gwget_pref.download_dir != NULL);

	dnd_type = GPOINTER_TO_UINT(data);
	
	drop_ok = FALSE;
	if (dnd_type==TARGET_URI_LIST) {
		GnomeVFSURI *vfs_uri;
		files = gnome_vfs_uri_list_parse (seldata->data);
		vfs_uri = files->data;
		file = gnome_vfs_uri_to_string(vfs_uri,GNOME_VFS_URI_HIDE_NONE);
		drop_ok = TRUE;
	} else if (dnd_type==TARGET_NETSCAPE_URL) {
		file=((gchar *) (seldata->data));
		drop_ok = TRUE;
	} else 	{
		gtk_drag_finish(context, FALSE, TRUE, time);
	}

	if (drop_ok && gwget_pref.ask_save_each_dl) {
		create_new_window_with_url (file);
	} else if (drop_ok && !gwget_pref.ask_save_each_dl) {
		gwgetdata = gwget_data_create(file, gwget_pref.download_dir);
		if (gwgetdata) {
			new_download(gwgetdata);
			gwget_data_start_download(gwgetdata);
		} else {
			run_dialog(_("Error starting the download"), _("There was an unexpected error starting the download"));
		}
		gtk_drag_finish(context, TRUE, FALSE, time);
	}
	
}

static void
gwget_gconf_notify_toolbar(GConfClient *client,
				   			guint        cnxn_id,
		   					GConfEntry  *entry,
		   					gpointer     user_data)
{
	GConfValue  *value;
	GtkWidget *toolbar;
	gchar *toolbar_setting;
	
	value = gconf_entry_get_value (entry);
	
	toolbar = glade_xml_get_widget(xml,"toolbar1");
	toolbar_setting = (gchar *)gconf_value_get_string(value);
	
	if (!strcmp(toolbar_setting,"icons")) {
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_ICONS);
	}
	
	if (!strcmp(toolbar_setting,"both")) {
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_BOTH);
	}
	
	if (!strcmp(toolbar_setting,"both-horiz")) {
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_BOTH_HORIZ);
	}
	
	if (!strcmp(toolbar_setting,"text")) {
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_TEXT);
	}

	g_free(toolbar_setting);
}

static void 
show_prefered_columns(void)
{	
	GtkWidget *treev,*column,*checkitem;
	gchar *xml_file=NULL;
	
	if (!xml_pref) {
		xml_file=g_build_filename(DATADIR,"preferences.glade",NULL);
		xml_pref = glade_xml_new(xml_file,NULL,NULL);
		glade_xml_signal_autoconnect(xml_pref);
	}
	
	treev = glade_xml_get_widget(xml,"treeview1");
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),CURRENTSIZE_COLUMN-2);
	checkitem=glade_xml_get_widget(xml_pref,"check_actual_size");
	if (gwget_pref.view_actual_size) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),FALSE);
	}
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),TOTALSIZE_COLUMN-2);
	checkitem=glade_xml_get_widget(xml_pref,"check_total_size");
	if (gwget_pref.view_total_size) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),FALSE);
	}
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),PERCENTAGE_COLUMN-2);
	checkitem=glade_xml_get_widget(xml_pref,"check_percentage");
	if (gwget_pref.view_percentage) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem), TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem), FALSE);
	}
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),ELAPSETIME_COLUMN-3);
	checkitem=glade_xml_get_widget(xml_pref,"check_elapse_time");
	if (gwget_pref.view_elapse_time) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),FALSE);
	}
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),REMAINTIME_COLUMN-5);
	checkitem=glade_xml_get_widget(xml_pref,"check_rem_time");
	if (gwget_pref.view_rem_time) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	}
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),SPEED_COLUMN-7);
	checkitem=glade_xml_get_widget(xml_pref,"check_down_speed");
	if (gwget_pref.view_down_speed) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem), TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem), FALSE);
	}
	
	checkitem = glade_xml_get_widget(xml_pref,"limit_speed_check");
	if (gwget_pref.limit_speed) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),FALSE);
	}
	checkitem = glade_xml_get_widget(xml_pref,"limit_speed_spin");
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(checkitem), gwget_pref.max_speed);


	checkitem = glade_xml_get_widget(xml_pref,"limit_simultaneousdownloads_check");
	if (gwget_pref.limit_simultaneousdownloads) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),FALSE);
	}
	checkitem = glade_xml_get_widget(xml_pref,"limit_simultaneousdownloads_spin");
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(checkitem), gwget_pref.max_simultaneousdownloads);

}

/*
 * This is called as a GTK singal handler , so the return value tells
 * the singal was not handled and should propagate further
 */
gboolean
gwget_remember_window_size_and_position(void)
{
	GtkWidget *main_window;
	GtkAllocation *allocation;
	gint position_x,position_y;

	/* Remember the size of the window */
	main_window=glade_xml_get_widget(xml,"main_window");
	allocation= &(GTK_WIDGET (main_window)->allocation);
	gconf_client_set_int (gconf_client, 
			      "/apps/gwget2/default_width",
			      allocation->width,
			      NULL);
	gconf_client_set_int (gconf_client, 
			      "/apps/gwget2/default_height",
			      allocation->height,
			      NULL);
	
	/* Remember the position */
	gtk_window_get_position(GTK_WINDOW(main_window), &position_x, &position_y);
	gconf_client_set_int (gconf_client,"/apps/gwget2/position_x",position_x,NULL);
	gconf_client_set_int (gconf_client,"/apps/gwget2/position_y",position_y,NULL);
	
	return FALSE;
}

/*
 * The return value is used in gwget_quit
 */
gboolean 
gwget_remember_downloads(void)
{
	gchar *url,*key;
	GwgetData *gwgetdata;
	GtkTreeIter iter;
	gint i,length;
	gboolean running = FALSE;
	
	/* calculate the number of items in the treeview */
	length=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),NULL);
	
	/* Save the number of current downloads in the treev */
	/* When load again we can known the number of directories to load */
	gconf_client_set_int(gconf_client,"/apps/gwget2/n_downloads",length,NULL);
		
	gtk_tree_model_get_iter_root(model,&iter);
	/* Save current downloads in GConf */
	/* Calculate if there are any dl in retriving state */
	for (i=0;i<length;i++) {
		gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
		gwgetdata=g_object_get_data(G_OBJECT(model),url);
	
		key=g_strdup_printf("/apps/gwget2/downloads_data/%d",i);
		gconf_client_add_dir(gconf_client,
				     key,
				     GCONF_CLIENT_PRELOAD_NONE,
				     NULL);
		key=g_strdup_printf("/apps/gwget2/downloads_data/%d/url",i);
		gconf_client_set_string(gconf_client,key,gwgetdata->url,NULL);
		
		key=g_strdup_printf("/apps/gwget2/downloads_data/%d/filename",i);
		gconf_client_set_string(gconf_client,key,gwgetdata->filename,NULL);	
	
		key=g_strdup_printf("/apps/gwget2/downloads_data/%d/dir",i);
		gconf_client_set_string(gconf_client,key,gwgetdata->dir,NULL);	
		
		key=g_strdup_printf("/apps/gwget2/downloads_data/%d/state",i);
		gconf_client_set_int(gconf_client,key,gwgetdata->state,NULL);

		key=g_strdup_printf ("/apps/gwget2/downloads_data/%d/total_size",i);
		gconf_client_set_int (gconf_client,key,gwgetdata->total_size,NULL);		
		
		if (gwgetdata->log_tag != -1) 
		{
			running=TRUE;
		}
		
		gtk_tree_model_iter_next(model,&iter);
	}

	return running;
}

void
gwget_quit(void)
{
	gboolean running;
	gint response;
	
	gwget_remember_window_size_and_position();
	running = gwget_remember_downloads();
	
	if (running) {
		response = run_dialog(_("Cancel current downloads?"),
				      _("There is at least one active download left. Really cancel all running transfers?"));
		if (response == GTK_RESPONSE_OK) {
			stop_all_downloads();
			gtk_main_quit();
		}
	} else {
		gtk_main_quit();
	}
}

