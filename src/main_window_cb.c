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
#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <signal.h>
#include <libgnome/gnome-url.h>
#include <libgnome/gnome-program.h>
#include <libgnome/gnome-init.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include "main_window.h"
#include "main_window_cb.h"
#include "new_window.h"
#include "gwget_data.h"
#include "utils.h"
#include "systray.h"


void 
on_stop_button_clicked (GtkWidget *widget, gpointer data) 
{
	GwgetData* gwgetdata;
		
	gwgetdata = gwget_data_get_selected();
	
	if (gwgetdata) {
		gwget_data_stop_download(gwgetdata);
	}
}

gboolean
on_treeview1_button_press_event(GtkWidget *widget, GdkEventButton *event,gpointer user_data)
{
	GtkWidget *popup,*treev;
	GdkEventButton *event_button;
	GtkTreeSelection *select;
	GtkTreeIter iter;
    GtkTreeModel *model;
	GwgetData *gwgetdata;
	gchar *uri;
	GError *err = NULL; 
	
	treev=glade_xml_get_widget(xml,"treeview1");
	select=gtk_tree_view_get_selection(GTK_TREE_VIEW(treev));
		
	if (event->type == GDK_BUTTON_PRESS) {
		event_button = (GdkEventButton *) event;
		if (event->button==3 && gtk_tree_selection_get_selected (select, &model, &iter)) {
			gwgetdata=gwget_data_get_selected();
			if (gwget_data_run(gwgetdata)) {
				gtk_widget_set_sensitive(glade_xml_get_widget(xml,"pause_download"),TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget(xml,"continue_download"),FALSE);
				gtk_widget_set_sensitive(glade_xml_get_widget(xml,"cancel_download"),TRUE);
			} else {
				if (gwgetdata->state==DL_COMPLETED) {
					gtk_widget_set_sensitive(glade_xml_get_widget(xml,"continue_download"),FALSE);
					gtk_widget_set_sensitive(glade_xml_get_widget(xml,"pause_download"),FALSE);
					gtk_widget_set_sensitive(glade_xml_get_widget(xml,"cancel_download"),FALSE);
				} else {
					gtk_widget_set_sensitive(glade_xml_get_widget(xml,"continue_download"),TRUE);
					gtk_widget_set_sensitive(glade_xml_get_widget(xml,"pause_download"),FALSE);
				}
			}
			popup=glade_xml_get_widget(xml,"popup1");
			gtk_menu_popup (GTK_MENU(popup), NULL, NULL, NULL, NULL, 
							event_button->button, event_button->time);
			return TRUE;
		}
	}
	
	/* Double click */
	/* Open the default gnome application */
	/* Code from: http://gnomesupport.org/forums/viewtopic.php?t=3852&highlight=default+application */
	if (event->type == GDK_2BUTTON_PRESS) {
		event_button = (GdkEventButton *) event;
		if (event->button==1 && gtk_tree_selection_get_selected (select, &model, &iter)) {
			gwgetdata=gwget_data_get_selected();
			/* If the download is in error state, show the error message in a dialog */
			if (gwgetdata->error) {
				run_dialog(_("Error in download"),gwgetdata->error_msg);
			} else {
				uri = gnome_vfs_make_uri_from_input_with_dirs (gwgetdata->local_filename,
                                                 GNOME_VFS_MAKE_URI_DIR_CURRENT);
				if (!gnome_url_show (uri, &err)) {
					run_dialog(_("Error opening file"),_("Couldn't open the file"));
				}
				return TRUE;
			}	
		}
	}
	return FALSE;			   
}

void 
stop_all_downloads(void)
{
	GwgetData* gwgetdata;
	GtkTreeIter iter;
	gint length,i;
	gchar *url;
	
	length=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),NULL);
	gtk_tree_model_get_iter_root(model,&iter);
	for (i=0;i<length;i++) {
		gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
		gwgetdata=g_object_get_data(G_OBJECT(model),url);
			
		if (gwget_data_run(gwgetdata)) {
				kill(gwgetdata->wget_pid,SIGINT);
		}
	}
}

void 
on_quit1_activate(GtkWidget *widget, gpointer data) 
{
	gwget_quit();
}


void 
on_about1_activate(GtkWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	GdkPixbuf *pixbuf = NULL;
	gchar *logo_file;
	gchar *copy_text = "Copyright © 2004 David Sedeño Fernández";
	gchar *about_text = _("Download Manager for Gnome2.");
	gchar *authors[] = { 
			"David Sedeño Fernández <david@alderia.com>",
			NULL
	};

	gchar *translator_credits = _("translator_credits");

	if (about != NULL ) 
	{
		gdk_window_show (about->window);
		gdk_window_raise (about->window);
		return;
	}

	logo_file=g_strdup_printf("%s/%s",DATADIR,"gwget-large.png");
	pixbuf = gdk_pixbuf_new_from_file (logo_file, NULL);

	about = gnome_about_new (_("Gwget"), VERSION,
							copy_text,
							about_text,
							(const char **)authors,
							(const char **)NULL,
							(const char *)translator_credits,
							pixbuf);

	if (pixbuf != NULL) {
		g_object_unref (pixbuf);
	}
	
	g_signal_connect (G_OBJECT (about), "destroy", 
						G_CALLBACK (gtk_widget_destroyed), &about);
	                                                                                                                             
	gtk_widget_show (about);

}


void 
on_button_new_clicked(GtkWidget *widget, gpointer data) 
{
	create_new_window();
}
	
void 
new_download(GwgetData* gwgetdata) {
	GtkTreeIter iter;
	GtkIconTheme *theme;
	GtkIconInfo *icon_info;
	GdkPixbuf *pixbuf;
	char *mime, *icon_name;
	int width = 16, height = 16;
	
	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,URL_COLUMN,gwgetdata->url,
						CURRENTSIZE_COLUMN,gwgetdata->cur_size, 
						FILENAME_COLUMN, gwgetdata->filename,
	    				-1);
	
	gwgetdata->file_list=iter; 
	
	g_object_set_data(G_OBJECT(model),gwgetdata->url,gwgetdata);
	
	downloads = g_list_append(downloads,gwgetdata);
	
	mime = (gchar *)gnome_vfs_mime_type_from_name(gwgetdata->local_filename);
	theme = gtk_icon_theme_get_default ();
	icon_name = gnome_icon_lookup (theme, NULL, NULL, NULL, NULL,
									mime, GNOME_ICON_LOOKUP_FLAGS_NONE, NULL);
	g_free (mime);
	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);
	width *= 2;
	height *= 2;
	
	icon_info = gtk_icon_theme_lookup_icon (theme, icon_name, width, height);
	g_free (icon_name);
	if (icon_info == NULL) return;
	
	icon_name = g_strdup (gtk_icon_info_get_filename (icon_info));
	gtk_icon_info_free (icon_info);
	
	pixbuf = gdk_pixbuf_new_from_file_at_size (icon_name, width, height, NULL);
	gtk_list_store_set (GTK_LIST_STORE (model),
						&iter, IMAGE_COLUMN, pixbuf, -1);
	if (pixbuf)
		g_object_unref (pixbuf);
	
	
}


void 
on_boton_pref_clicked(GtkWidget *widget, gpointer data)
{
	gchar *xml_file = NULL;
	GtkWidget *window = NULL,*entry=NULL, *checkbutton=NULL;
	
	
	if (!xml_pref) {
		xml_file=g_build_filename(DATADIR,"preferences.glade",NULL);
		xml_pref = glade_xml_new(xml_file,NULL,NULL);
		glade_xml_signal_autoconnect(xml_pref);
	}
		
	window = glade_xml_get_widget (xml_pref, "pref_window");
	if (gwget_pref.download_dir!=NULL) {
		entry = glade_xml_get_widget(xml_pref,"save_in_entry");
		gtk_entry_set_text(GTK_ENTRY(entry),gwget_pref.download_dir);
	}
	entry = glade_xml_get_widget(xml_pref,"num_retries_entry");
	gtk_entry_set_text(GTK_ENTRY(entry),g_strdup_printf("%d",gwget_pref.num_retries));
	
	checkbutton=glade_xml_get_widget(GLADE_XML(xml_pref),"no_create_directories");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.no_create_directories);
	checkbutton=glade_xml_get_widget(GLADE_XML(xml_pref),"follow_relative");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.follow_relative);

	checkbutton=glade_xml_get_widget(GLADE_XML(xml_pref),"convert_links");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.convert_links);

	checkbutton=glade_xml_get_widget(GLADE_XML(xml_pref),"dl_page_requisites");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.dl_page_requisites);

	checkbutton=glade_xml_get_widget(GLADE_XML(xml_pref),"max_depth");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(checkbutton), (gdouble)gwget_pref.max_depth);

	gtk_widget_show(window);
		
	return;
	
}

void 
on_pref_cancel_button_clicked(GtkWidget *widget,gpointer data)
{
	GtkWidget *pref_window = NULL;
	
	pref_window = glade_xml_get_widget(xml_pref,"pref_window");
	gtk_widget_hide(pref_window);
}
	
void 
on_pref_ok_button_clicked(GtkWidget *widget,gpointer data)
{
	GtkWidget *save_in = NULL, *pref_window = NULL,*num_retries=NULL,*resume;
	GtkWidget *no_create_directories = NULL;
	GtkWidget *follow_relative = NULL;	
	GtkWidget *convert_links = NULL;
	GtkWidget *dl_page_requisites = NULL;
	GtkWidget *max_depth=NULL;

	save_in=glade_xml_get_widget(xml_pref,"save_in_entry");
	gwget_pref.download_dir=g_strdup(gtk_entry_get_text(GTK_ENTRY(save_in)));
	
	num_retries=glade_xml_get_widget(xml_pref,"num_retries_entry");
	gwget_pref.num_retries=atoi(gtk_entry_get_text(GTK_ENTRY(num_retries)));
	
	resume=glade_xml_get_widget(xml_pref,"resume_at_start");
	gwget_pref.resume_at_start=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(resume));
	
	pref_window = glade_xml_get_widget(xml_pref,"pref_window");
	gtk_widget_hide(pref_window);
	
	gconf_client_set_string(gconf_client,"/apps/gwget2/download_dir",
							g_strdup(gtk_entry_get_text(GTK_ENTRY(save_in))),NULL);
	gconf_client_set_int(gconf_client,"/apps/gwget2/num_retries",
						atoi(gtk_entry_get_text(GTK_ENTRY(num_retries))),NULL);
	gconf_client_set_bool(gconf_client,"/apps/gwget2/resume_at_start",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(resume)),NULL);
	
	/* Recursivity */
	no_create_directories  = glade_xml_get_widget(GLADE_XML(xml_pref),"no_create_directories");
	gwget_pref.no_create_directories = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(no_create_directories));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/no_create_directories",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(no_create_directories)),NULL);
	
	/* Follow relative links only */
	follow_relative = glade_xml_get_widget(GLADE_XML(xml_pref),"follow_relative");
	gwget_pref.follow_relative = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(follow_relative));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/follow_relative",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(follow_relative)), NULL);
	
	/* Convert links */
	convert_links = glade_xml_get_widget(GLADE_XML(xml_pref),"convert_links");
	gwget_pref.follow_relative = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(convert_links));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/convert_links",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(convert_links)), NULL);
	/* Download page requisites */
	dl_page_requisites= glade_xml_get_widget(GLADE_XML(xml_pref),"dl_page_requisites");
	gwget_pref.follow_relative = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dl_page_requisites));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/dl_page_requisites",	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dl_page_requisites)), NULL);
	/* Max Depth */
	max_depth=glade_xml_get_widget(GLADE_XML(xml_pref),"max_depth");
	gwget_pref.max_depth=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(max_depth));
	gconf_client_set_int(gconf_client,"/apps/gwget2/max_depth",gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(max_depth)), NULL);
	
	/* Column listing */
	gwget_pref.view_file_type=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_file_type")));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_file_type",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_file_type"))),NULL);
	
	gwget_pref.view_actual_size=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_actual_size")));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_actual_size",
	 					  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_actual_size"))),NULL);

	gwget_pref.view_total_size=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_total_size")));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_total_size",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_total_size"))),NULL);
	
	gwget_pref.view_percentage=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_percentage")));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_percentage",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_percentage"))),NULL);


	gwget_pref.view_elapse_time=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_elapse_time")));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_elapse_time",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_elapse_time"))),NULL);

	gwget_pref.view_rem_time=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_rem_time")));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_rem_time",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_rem_time"))),NULL);

	gwget_pref.view_down_speed=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_down_speed")));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_down_speed",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(GLADE_XML(xml_pref),"check_down_speed"))),NULL);

}

void
on_browse_save_in_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *filesel = NULL;
	GtkWidget *save_in;
	save_in = glade_xml_get_widget(xml_pref,"save_in_entry");
	
	filesel = gtk_file_chooser_dialog_new  (_("Select Folder"),
											NULL,
											GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
											GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      						GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
											NULL);
	
	if (gtk_dialog_run (GTK_DIALOG (filesel)) == GTK_RESPONSE_ACCEPT) {
		char *directory;
		
		directory = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
		gtk_entry_set_text(GTK_ENTRY(save_in),directory);
		
		g_free(directory);
	}
	
	gtk_widget_destroy(filesel);

}

void 
on_popup_pause_button_clicked(GtkWidget *widget, gpointer data)
{
	GwgetData *gwgetdata;
	
	gwgetdata = gwget_data_get_selected();

	if (gwgetdata) {
		gwget_data_stop_download(gwgetdata);
	}
}

void 
on_popup_continue_activate(GtkWidget *widget, gpointer data)
{
	GwgetData *gwgetdata;
	
	gwgetdata = gwget_data_get_selected();
	
	if (gwgetdata) {
		gwget_data_start_download(gwgetdata);
	} else {
		printf("NULL\n");
	}
}

void 
on_cancel_download_activate(GtkWidget *widget,gpointer data)
{
	gint response;
	GwgetData *gwgetdata;
	gchar *msg;
	
	gwgetdata = gwget_data_get_selected();
	
	if (gwgetdata) 
	{
		msg = g_strdup_printf(_("Really cancel current download? (Url: %s)"),gwgetdata->url);
		response = run_dialog(_("Cancel download?"),_(msg));
		if (response==GTK_RESPONSE_OK) {
			gwget_data_stop_download(gwgetdata); 
			unlink (gwgetdata->local_filename);
    		gwgetdata->total_time = 0;
    		gwget_data_set_state (gwgetdata, DL_COMPLETED);
			gtk_list_store_remove(GTK_LIST_STORE(model),&gwgetdata->file_list);
			downloads=g_list_remove(downloads,gwgetdata);
			printf("N: %d\n",g_list_length(downloads));
			gwget_data_free(gwgetdata);
			
		}
	}
}


/* Remove completed downloads (popup) */
void 
on_remove_completed_activate(GtkWidget *widget, gpointer data)
{
	gint response;
	GwgetData *gwgetdata;
	GtkTreeIter iter;
	gint length,i;
	gchar *url;
	
	response = run_dialog(_("Remove completed"),_("Really remove completed downloads from the list?"));
	if (response == GTK_RESPONSE_OK) {
		length=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),NULL);
		gtk_tree_model_get_iter_root(model,&iter);
		for (i=0;i<length;i++) {
			gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
			gwgetdata=g_object_get_data(G_OBJECT(model),url);
			
			if (gwgetdata->state==DL_COMPLETED) {
				gtk_list_store_remove(GTK_LIST_STORE(model),&iter);
				downloads=g_list_remove(downloads,gwgetdata);
			} else {
				gtk_tree_model_iter_next(model,&iter);
			}
		}
	}
				
}

/* Remove not running downloads (popup) */
void 
on_remove_notrunning_activate(GtkWidget *widget, gpointer data)
{
	gint response;
	GwgetData *gwgetdata;
	GtkTreeIter iter;
	gint length,i;
	gchar *url;
	
	response = run_dialog(_("Remove not running"),_("Really remove not running downloads from the list?"));
	if (response == GTK_RESPONSE_OK) {
		length=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),NULL);
		gtk_tree_model_get_iter_root(model,&iter);
		for (i=0;i<length;i++) {
			gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
			printf("URL:%s\n",url);
			gwgetdata=g_object_get_data(G_OBJECT(model),url);
			
			if (gwgetdata->state!=DL_RETRIEVING) {
				gtk_list_store_remove(GTK_LIST_STORE(model),&iter);
				downloads=g_list_remove(downloads,gwgetdata);
			} else {
				gtk_tree_model_iter_next(model,&iter);
			}
		}
	}
				
}

/* Remove all downloads (popup) */
void 
on_remove_all_activate(GtkWidget *widget, gpointer data)
{
	gint response;
	GwgetData *gwgetdata;
	GtkTreeIter iter;
	gint length,i;
	gchar *url;
	
	response = run_dialog(_("Remove all"),_("Really remove all downloads from the list?"));
	if (response == GTK_RESPONSE_OK) {
		length=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),NULL);
		gtk_tree_model_get_iter_root(model,&iter);
		for (i=0;i<length;i++) {
			gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
			printf("URL:%s\n",url);
			gwgetdata=g_object_get_data(G_OBJECT(model),url);
			/* If it's running we must stop it */
			/* because the function that update the info will */
			/* be continue trying to update the info, so segfault! */
			if (gwgetdata->state==DL_RETRIEVING) {
				gwget_data_stop_download(gwgetdata);
			}
			
			gtk_list_store_remove(GTK_LIST_STORE(model),&iter);
			downloads=g_list_remove(downloads,gwgetdata);
		}
	}
				
}

void 
on_view_toolbar_activate(GtkWidget *widget,gpointer data)
{
	GtkWidget *toolbar,*menu_item;
	gboolean state;
	
	toolbar=glade_xml_get_widget(GLADE_XML(xml),"bonobotoolbar");
	menu_item=glade_xml_get_widget(GLADE_XML(xml),"view_toolbar");
	state = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item));
	
	if (!state) 
	{
		gtk_widget_hide(GTK_WIDGET(toolbar));	
	} else {
		gtk_widget_show(GTK_WIDGET(toolbar));
	}		
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_toolbar",state,NULL);
}

void 
on_properties_activate(GtkWidget *widget, gpointer data)
{
	GtkWidget *main_window,*properties,*url_txt,*local_file,*local_dir;
	GwgetData *gwgetdata;

	gwgetdata = gwget_data_get_selected();	
	
	main_window=glade_xml_get_widget(GLADE_XML(xml),"main_window");
	properties=glade_xml_get_widget(GLADE_XML(xml),"properties_window");
	gtk_window_set_transient_for(GTK_WINDOW(properties),GTK_WINDOW(main_window));
	url_txt=glade_xml_get_widget(GLADE_XML(xml),"url_text");
	gtk_label_set_text(GTK_LABEL(url_txt),gwgetdata->url);
	local_file=glade_xml_get_widget(GLADE_XML(xml),"local_file_text");
	gtk_label_set_text(GTK_LABEL(local_file),gwgetdata->filename);
	local_dir=glade_xml_get_widget(GLADE_XML(xml),"local_dir");
	gtk_label_set_text(GTK_LABEL(local_dir),gwgetdata->dir);
	gtk_widget_show(properties);
}

void
on_check_file_type_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev, *column;
	gboolean visible;
	
	treev = glade_xml_get_widget(xml,"treeview1");
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),IMAGE_COLUMN);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
}

void 
on_check_actual_size_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = glade_xml_get_widget(xml,"treeview1");
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),CURRENTSIZE_COLUMN-1);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
}

void 
on_check_total_size_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = glade_xml_get_widget(xml,"treeview1");
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),TOTALSIZE_COLUMN-1);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
	
}

void 
on_check_percentage_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = glade_xml_get_widget(xml,"treeview1");
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),PERCENTAGE_COLUMN-1);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
	
}

void 
on_check_elapse_time_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = glade_xml_get_widget(xml,"treeview1");
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),ELAPSETIME_COLUMN-2);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
	
}

void 
on_check_rem_time_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = glade_xml_get_widget(xml,"treeview1");
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),REMAINTIME_COLUMN-4);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
	
}

void 
on_check_down_speed_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = glade_xml_get_widget(xml,"treeview1");
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),SPEED_COLUMN-6);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
	
}
