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
#include "main_window.h"
#include "main_window_cb.h"
#include "gwget_data.h"
#include "custom-cell-renderer-progressbar.h"
#include "systray.h"



/* Global Preferences */
Preferences gwget_pref;

static void 
show_prefered_columns(void);

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
	GtkWidget *treev,*toolbar,*menu_item;
	GtkTreeSelection *select;
	

	if (!xml) {
		xml_file=g_build_filename(DATADIR,"gwget.glade",NULL);
		xml = glade_xml_new(xml_file,NULL,NULL);
	
		glade_xml_signal_autoconnect(xml);
	}
	
	window = glade_xml_get_widget(xml,"main_window");
	treev = glade_xml_get_widget(xml,"treeview1");
	model = create_model();
	gtk_tree_view_set_model(GTK_TREE_VIEW(treev),GTK_TREE_MODEL(model));
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treev));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
                
	/* add the columns titles to the tree view */
    add_columns (GTK_TREE_VIEW (treev));
	
	gconf_client = gconf_client_get_default();
	gconf_client_add_dir (gconf_client, "/apps/gwget", GCONF_CLIENT_PRELOAD_NONE,
                        NULL);
	
	/* gwget_pref.download_dir=NULL; */
	gwget_get_defaults_from_gconf();
	
	/* Drag and drop set up */
	gtk_drag_dest_set(GTK_WIDGET(window), 
					  GTK_DEST_DEFAULT_ALL | GTK_DEST_DEFAULT_HIGHLIGHT,
					  dragtypes, sizeof(dragtypes) / sizeof(dragtypes[0]),
                      GDK_ACTION_COPY);
						
	g_signal_connect(G_OBJECT(window), "drag-data-received",
					 G_CALLBACK(on_treeview_drag_received),
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
	gtk_widget_show(window);
	toolbar = glade_xml_get_widget(xml,"bonobotoolbar"); 
	menu_item=glade_xml_get_widget(GLADE_XML(xml),"view_toolbar");
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item))) {
		gtk_widget_hide(GTK_WIDGET(toolbar));
	} else {
		gtk_widget_show(GTK_WIDGET(toolbar));
	}
	
	systray_load();
}

void 
gwget_get_defaults_from_gconf(void)
{
	gint num_dl,i;
	GwgetData *data;
	gchar *key,*url,*dir;
	DlState state;
	gint default_width, default_height;
	
	gwget_pref.download_dir=gconf_client_get_string(gconf_client,"/apps/gwget/download_dir",NULL);
	gwget_pref.num_retries=gconf_client_get_int(gconf_client,"/apps/gwget/num_retries",NULL);
	gwget_pref.resume_at_start=gconf_client_get_bool(gconf_client,"/apps/gwget/resume_at_start",NULL);
	gwget_pref.no_create_directories=gconf_client_get_bool(gconf_client,"/apps/gwget/no_create_directories",NULL);
	gwget_pref.follow_relative=gconf_client_get_bool(gconf_client,"/apps/gwget/follow_relative",NULL);
	gwget_pref.convert_links = gconf_client_get_bool(gconf_client,"/apps/gwget/convert_links",NULL);	
	gwget_pref.dl_page_requisites = gconf_client_get_bool(gconf_client,"/apps/gwget/dl_page_requisites",NULL);
	gwget_pref.max_depth=gconf_client_get_int(gconf_client,"/apps/gwget/max_depth",NULL);
	gwget_pref.view_actual_size=gconf_client_get_bool(gconf_client,"/apps/gwget/view_actual_size",NULL);
	gwget_pref.view_total_size=gconf_client_get_bool(gconf_client,"/apps/gwget/view_total_size",NULL);
	gwget_pref.view_percentage=gconf_client_get_bool(gconf_client,"/apps/gwget/view_percentage",NULL);
	gwget_pref.view_elapse_time=gconf_client_get_bool(gconf_client,"/apps/gwget/view_elapse_time",NULL);
	gwget_pref.view_down_speed=gconf_client_get_bool(gconf_client,"/apps/gwget/view_down_speed",NULL);
	gwget_pref.view_toolbar=gconf_client_get_bool(gconf_client,"/apps/gwget/view_toolbar",NULL);
	
	num_dl=gconf_client_get_int(gconf_client,"/apps/gwget/n_downloads",NULL);
	for (i=0;i<num_dl;i++) {
		key=g_strdup_printf("/apps/gwget/downloads_data/%d/url",i);
		url=gconf_client_get_string(gconf_client,key,NULL);
		key=g_strdup_printf("/apps/gwget/downloads_data/%d/dir",i);
		dir=gconf_client_get_string(gconf_client,key,NULL);
		data=gwget_data_create(url,dir);
		key=g_strdup_printf("/apps/gwget/downloads_data/%d/state",i);
		state=gconf_client_get_int(gconf_client,key,NULL); 
		new_download(data);
		gwget_data_set_state(data,state); 
		if (gwget_pref.resume_at_start && data->state!=DL_COMPLETED) {
			gwget_data_start_download(data);
		}
	}
	show_prefered_columns();
		
	/* Default width and height */
	default_width=gconf_client_get_int (gconf_client,"/apps/gwget/default_width",NULL);
	default_height=gconf_client_get_int (gconf_client,"/apps/gwget/default_height",NULL);
	gtk_window_resize (GTK_WINDOW (glade_xml_get_widget(xml,"main_window")),default_width,default_height);
	
	/* Default position */
	gtk_window_move(GTK_WINDOW(glade_xml_get_widget(xml,"main_window")),
					gconf_client_get_int (gconf_client,"/apps/gwget/position_x",NULL),
					gconf_client_get_int (gconf_client,"/apps/gwget/position_y",NULL)
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
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("File Name"),
													     renderer,
						     							"text",
						     							FILENAME_COLUMN,
						     							NULL);
	gtk_tree_view_column_set_sort_column_id (column, FILENAME_COLUMN);
	gtk_tree_view_append_column (treeview, column);

	/* State Column */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("State"),
														renderer,
						     							"text",
														STATE_COLUMN,
														NULL);
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
on_treeview_drag_received (GtkWidget * widget, GdkDragContext * context, int x,
							int y, GtkSelectionData * seldata, guint info,	
							guint time, gpointer data)
{
	gchar *file;
	GwgetData *gwgetdata;
	
	dnd_type = GPOINTER_TO_UINT(data);
	file=((gchar *) (seldata->data));
      
	if (dnd_type==TARGET_URI_LIST || dnd_type==TARGET_NETSCAPE_URL) {
		gwgetdata=gwget_data_create(file,gwget_pref.download_dir);
		new_download(gwgetdata);
		gwget_data_start_download(gwgetdata);
		gtk_drag_finish(context, TRUE, FALSE, time);
	} else 	{
		gtk_drag_finish(context, FALSE, TRUE, time);
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
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),CURRENTSIZE_COLUMN-1);
	checkitem=glade_xml_get_widget(xml_pref,"check_actual_size");
	if (gwget_pref.view_actual_size) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),FALSE);
	}
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),TOTALSIZE_COLUMN-1);
	checkitem=glade_xml_get_widget(xml_pref,"check_total_size");
	if (gwget_pref.view_total_size) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),FALSE);
	}
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),PERCENTAGE_COLUMN-1);
	checkitem=glade_xml_get_widget(xml_pref,"check_percentage");
	if (gwget_pref.view_percentage) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem), TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem), FALSE);
	}
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),ELAPSETIME_COLUMN-2);
	checkitem=glade_xml_get_widget(xml_pref,"check_elapse_time");
	if (gwget_pref.view_elapse_time) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),FALSE);
	}
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),REMAINTIME_COLUMN-4);
	checkitem=glade_xml_get_widget(xml_pref,"check_rem_time");
	if (gwget_pref.view_rem_time) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem),TRUE);
	}
	
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),SPEED_COLUMN-6);
	checkitem=glade_xml_get_widget(xml_pref,"check_down_speed");
	if (gwget_pref.view_down_speed) {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem), TRUE);
	} else {
		gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkitem), FALSE);
	}
	
}


void
gwget_quit(void)
{
	gint response;
	gchar *url,*key;
	GwgetData *gwgetdata;
	GtkTreeIter iter;
	gint i,length,position_x,position_y;
	gboolean running; 
	GtkWidget *main_window;
	GtkAllocation *allocation;
	
	/* calculate the number of items in the treeview */
	length=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),NULL);
	
	/* Save the number of current downloads in the treev */
	/* When load again we can known the number of directories to load */
	gconf_client_set_int(gconf_client,"/apps/gwget/n_downloads",length,NULL);
		
	gtk_tree_model_get_iter_root(model,&iter);
	/* Safe current downloads in GConf */
	/* Calculate if there are any dl in retriving state */
	running = FALSE;
	for (i=0;i<length;i++) {
		gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
		gwgetdata=g_object_get_data(G_OBJECT(model),url);
	
		key=g_strdup_printf("/apps/gwget/downloads_data/%d",i);
		gconf_client_add_dir(gconf_client,key,
							 GCONF_CLIENT_PRELOAD_NONE,NULL);
		key=g_strdup_printf("/apps/gwget/downloads_data/%d/url",i);
		gconf_client_set_string(gconf_client,key,gwgetdata->url,NULL);
		
		key=g_strdup_printf("/apps/gwget/downloads_data/%d/filename",i);
		gconf_client_set_string(gconf_client,key,gwgetdata->filename,NULL);	
	
		key=g_strdup_printf("/apps/gwget/downloads_data/%d/dir",i);
		gconf_client_set_string(gconf_client,key,gwgetdata->dir,NULL);	
		
		key=g_strdup_printf("/apps/gwget/downloads_data/%d/state",i);
		gconf_client_set_int(gconf_client,key,gwgetdata->state,NULL);	
		
		if (gwgetdata->state==DL_RETRIEVING) 
		{
			running=TRUE;
		}
		
		gtk_tree_model_iter_next(model,&iter);
	}
	
	/* Remember the size of the window */
	main_window=glade_xml_get_widget(xml,"main_window");
	allocation= &(GTK_WIDGET (main_window)->allocation);
	gconf_client_set_int (gconf_client, "/apps/gwget/default_width",allocation->width,NULL);
	gconf_client_set_int (gconf_client, "/apps/gwget/default_height",allocation->height,NULL);
	
	/* Remember the position */
	gtk_window_get_position(GTK_WINDOW(main_window), &position_x,&position_y);
	gconf_client_set_int (gconf_client,"/apps/gwget/position_x",position_x,NULL);
	gconf_client_set_int (gconf_client,"/apps/gwget/position_y",position_y,NULL);
	
	
	if (running) {
		response = run_dialog(_("Cancel current downloads?"),_("There is at least one active download left. Really cancel all running transfers?"));
		if (response == GTK_RESPONSE_OK) {
			stop_all_downloads();
			gtk_main_quit();
		}
	} else {
		gtk_main_quit();
	}
}
