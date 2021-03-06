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
#include <gconf/gconf-client.h>
#include <signal.h>

#include "main_window.h"
#include "main_window_cb.h"
#include "new_window.h"
#include "gwget_data.h"
#include "utils.h"
#include "systray.h"
#include "md5.h"

#define MD5BUFSIZE 512

static void inform_limit_speed_change(void);

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
	GFile *location; 
	
	treev=GTK_WIDGET (gtk_builder_get_object(builder,"treeview1"));
	select=gtk_tree_view_get_selection(GTK_TREE_VIEW(treev));

	/* Right click - Show popup menu */
	if (event->type == GDK_BUTTON_PRESS) {
		event_button = (GdkEventButton *) event;
		if (event->button==3 && gtk_tree_selection_get_selected (select, &model, &iter)) {
			GtkTreeSelection *selection;
			GtkTreePath *path;

			/* Select right-clicked line */
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treev));
			if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treev),
                                             (gint) event->x, 
                                             (gint) event->y,
                                             &path, NULL, NULL, NULL))
			{
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
				gtk_tree_path_free(path);
			}

			popup=GTK_WIDGET (gtk_builder_get_object(builder,"popup1"));
			gtk_menu_popup (GTK_MENU(popup), NULL, NULL, NULL, NULL, 
							event_button->button, event_button->time);
			gwget_data_set_popupmenu (gwget_data_get_selected());
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
				run_dialog_error(_("Error in download"),gwgetdata->error_msg);
			} else {
				if (gwgetdata->recursive) {
					location = g_file_parse_name (gwgetdata->dir); 
					uri = g_file_get_uri (location);
					g_object_unref (location);
				} else {
					location = g_file_new_for_commandline_arg (gwgetdata->local_filename);
					uri = g_file_get_uri (location);
					g_object_unref (location);
				}
				if (!gtk_show_uri (NULL, uri, GDK_CURRENT_TIME, &err)) {
					run_dialog_error(_("Error opening file"),_("Couldn't open the file"));
					return FALSE;
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
				gwget_data_stop_download(gwgetdata);
		}
		gtk_tree_model_iter_next(model,&iter);
	}

	gtk_window_set_title(GTK_WINDOW (gtk_builder_get_object(builder, "main_window")), _("Gwget - Download Manager"));
}

void
continue_all_downloads(void)
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
			
		if (!gwget_data_run(gwgetdata) && gwgetdata->state != DL_COMPLETED) {
				gwget_data_start_download(gwgetdata);
		}
		gtk_tree_model_iter_next(model,&iter);
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
	GdkPixbuf *pixbuf = NULL;
	gchar *logo_file;

	static const gchar *authors[] = { 
			"David Sedeño Fernández <david@alderia.com>",
			NULL
	};
		
	logo_file=g_strdup_printf("%s/%s",DATADIR,"gwget-large.png");
	pixbuf = gdk_pixbuf_new_from_file (logo_file, NULL);
	g_free (logo_file);

	gtk_show_about_dialog (GTK_WINDOW (gtk_builder_get_object(builder,"main_window")),
			       "name", _("Gwget"), 
			       "version", VERSION,
			       "copyright", "Copyright © 2004-2009 David Sedeño Fernández",
			       "comments", _("Download Manager for GNOME."),
			       "authors", authors,
			       "translator-credits", _("translator-credits"),
			       "logo", pixbuf,
			       NULL);

	if (pixbuf != NULL) {
		g_object_unref (pixbuf);
	}
}


void 
on_button_new_clicked(GtkWidget *widget, gpointer data) 
{
	create_new_window();
}
	
void 
new_download(GwgetData* gwgetdata) 
{
	GtkTreeIter iter;
	GtkTreeSelection *select;
	GtkIconTheme *theme;
	GtkIconInfo *icon_info;
	GdkPixbuf *pixbuf;
	gchar *content_type = NULL;
	gchar *size;
	int width = 16, height = 16;
	gdouble perc;
	
	gtk_list_store_append (GTK_LIST_STORE(model), &iter);

	size = get_readable_size (gwgetdata->cur_size);

	gtk_list_store_set (GTK_LIST_STORE(model), &iter,URL_COLUMN,gwgetdata->url,
						CURRENTSIZE_COLUMN, size, 
						FILENAME_COLUMN, gwgetdata->filename,
	    				-1);
	
  size = get_readable_size (gwgetdata->total_size);
	
	gtk_list_store_set (GTK_LIST_STORE(model), &iter, TOTALSIZE_COLUMN, size, -1);
	
	perc = gwgetdata->total_size==0?0:((gdouble)gwgetdata->cur_size*100)/(gdouble)gwgetdata->total_size ;
	gtk_list_store_set(GTK_LIST_STORE(model),&iter,
			PERCENTAGE_COLUMN,(gint)perc,-1);
		
	gwgetdata->file_list=iter; 

        select=gtk_tree_view_get_selection(GTK_TREE_VIEW(GTK_WIDGET (gtk_builder_get_object(builder,"treeview1"))));
	gtk_tree_selection_select_iter(select, &iter);
	
	g_object_set_data(G_OBJECT(model),gwgetdata->url,gwgetdata);
	
	theme = gtk_icon_theme_get_default ();
	if (!gwgetdata->recursive) {
		content_type = g_content_type_guess (gwgetdata->local_filename, NULL, 0, NULL);
		gwgetdata->icon_name = gnome_icon_lookup (theme, NULL, NULL, NULL, NULL,
	 							content_type, GNOME_ICON_LOOKUP_FLAGS_NONE, NULL);
	} else {
		gwgetdata->icon_name = g_strdup("gtk-refresh");
	}
	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);
	width *= 2;
	height *= 2;
	
	icon_info = gtk_icon_theme_lookup_icon (theme, gwgetdata->icon_name, width, height);
	if (icon_info == NULL) return;
	
	gwgetdata->icon_name = g_strdup (gtk_icon_info_get_filename (icon_info));
	gtk_icon_info_free (icon_info);
	
	pixbuf = gdk_pixbuf_new_from_file_at_size (gwgetdata->icon_name, width, height, NULL);
	gtk_list_store_set (GTK_LIST_STORE (model),
				&iter, IMAGE_COLUMN, pixbuf, -1);

	if (pixbuf)
		g_object_unref (pixbuf);
		
	g_free (content_type);
}


void 
on_boton_pref_clicked(GtkWidget *widget, gpointer data)
{
	gchar *builder_file = NULL;
	GtkWidget *window = NULL,*entry=NULL, *checkbutton=NULL, *main_window = NULL;
	GError* error = NULL;
	
	if (!builder_pref) {
		builder_file = g_build_filename(DATADIR,"preferences.ui",NULL);
		builder_pref = gtk_builder_new();
		if (!gtk_builder_add_from_file (builder_pref, builder_file, &error))
			{
				g_warning ("Couldn't load builder file: %s", builder_file);
				g_error_free (error);
			}
		gtk_builder_connect_signals(builder_pref, NULL);
	}
		
	main_window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
	window = GTK_WIDGET (gtk_builder_get_object (builder_pref, "pref_window"));
	entry = GTK_WIDGET (gtk_builder_get_object(builder_pref,"save_in_entry"));

	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(main_window));

	if (gwget_pref.download_dir && strlen(gwget_pref.download_dir)!=0) {
		gtk_entry_set_text(GTK_ENTRY(entry), gwget_pref.download_dir);
	} else {
		gtk_entry_set_text(GTK_ENTRY(entry), g_get_home_dir());
	}

        /* Put HTTP proxy values */

	if (gwget_pref.network_mode!=NULL) {
		if ( strcmp (gwget_pref.network_mode, "manual") == 0) {
			checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"manual_radio"));
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),TRUE);
		} else if ( strcmp (gwget_pref.network_mode, "default") == 0 ) {
			checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"default_radio"));
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),TRUE);
		} else {
			checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"direct_radio"));
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),TRUE);
		}
	} else {
			checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"direct_radio"));
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),TRUE);
	}
	

	if (gwget_pref.http_proxy!=NULL) {
		entry = GTK_WIDGET (gtk_builder_get_object(builder_pref,"http_proxy_entry"));
		gtk_entry_set_text(GTK_ENTRY(entry),gwget_pref.http_proxy);
		checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"proxy_uses_auth_radio"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.proxy_uses_auth);
		entry = GTK_WIDGET (gtk_builder_get_object(builder_pref,"proxy_user"));
		gtk_entry_set_text(GTK_ENTRY(entry),gwget_pref.proxy_user);
		entry = GTK_WIDGET (gtk_builder_get_object(builder_pref,"proxy_password"));
		gtk_entry_set_text(GTK_ENTRY(entry),gwget_pref.proxy_password);

	}
	
	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"http_proxy_port_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(checkbutton), (gdouble)gwget_pref.http_proxy_port);

	
	/* General */
	checkbutton = GTK_WIDGET (gtk_builder_get_object (builder_pref, "ask_save_each_dl_check"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton), gwget_pref.ask_save_each_dl);

	
	checkbutton = GTK_WIDGET (gtk_builder_get_object(builder_pref,"num_retries_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(checkbutton), (gdouble)gwget_pref.num_retries);

	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref, "resume_at_start"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.resume_at_start);

	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref, "open_after_dl"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.open_after_dl);
	
	/* Recursive */
	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"no_create_directories"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.no_create_directories);

	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"follow_relative"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.follow_relative);

	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"convert_links"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.convert_links);

	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"dl_page_requisites"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.dl_page_requisites);

	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"max_depth"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(checkbutton), (gdouble)gwget_pref.max_depth);

	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"limit_speed_check"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.limit_speed);

	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"limit_speed_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(checkbutton), (gdouble)gwget_pref.max_speed);

	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"limit_simultaneousdownloads_check"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),gwget_pref.limit_simultaneousdownloads);

	checkbutton=GTK_WIDGET (gtk_builder_get_object(builder_pref,"limit_simultaneousdownloads_spin"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(checkbutton), (gdouble)gwget_pref.max_simultaneousdownloads);

	gtk_widget_show(window);
		
	return;
	
}

void 
on_pref_cancel_button_clicked(GtkWidget *widget,gpointer data)
{
	GtkWidget *pref_window = NULL;
	
	pref_window = GTK_WIDGET (gtk_builder_get_object(builder_pref,"pref_window"));
	gtk_widget_hide(pref_window);
}

void 
on_pref_ok_button_clicked(GtkWidget *widget,gpointer data)
{
	GtkWidget *save_in = NULL, *ask_each_dl = NULL, *pref_window = NULL,*num_retries=NULL,*resume = NULL, *open_after_dl = NULL;
	GtkWidget *http_proxy = NULL, *http_proxy_port_spin = NULL, *proxy_user=NULL, *proxy_password=NULL;
	GtkWidget *no_create_directories = NULL;
	GtkWidget *follow_relative = NULL;	
	GtkWidget *convert_links = NULL;
	GtkWidget *dl_page_requisites = NULL;
	GtkWidget *max_depth=NULL, *limit_speed_check=NULL, *limit_speed_spin=NULL;
	GtkWidget *manual_radio=NULL, *direct_radio=NULL, *default_radio=NULL, *proxy_uses_auth_radio=NULL;
	GtkWidget *limit_simultaneousdownloads_check=NULL, *limit_simultaneousdownloads_spin=NULL;

	save_in=GTK_WIDGET (gtk_builder_get_object(builder_pref,"save_in_entry"));
	gwget_pref.download_dir=g_strdup(gtk_entry_get_text(GTK_ENTRY(save_in)));
	
	/* Set HTTP proxy values */
	
	http_proxy=GTK_WIDGET (gtk_builder_get_object(builder_pref,"http_proxy_entry"));
	gwget_pref.http_proxy=g_strdup(gtk_entry_get_text(GTK_ENTRY(http_proxy)));
	gconf_client_set_string(gconf_client,"/apps/gwget2/http_proxy",
							g_strdup(gtk_entry_get_text(GTK_ENTRY(http_proxy))),NULL);
	
	proxy_user=GTK_WIDGET (gtk_builder_get_object(builder_pref,"proxy_user"));
	gwget_pref.proxy_user=g_strdup(gtk_entry_get_text(GTK_ENTRY(proxy_user)));
	gconf_client_set_string(gconf_client,"/apps/gwget2/proxy_user",
							g_strdup(gtk_entry_get_text(GTK_ENTRY(proxy_user))),NULL);
							
	proxy_password=GTK_WIDGET (gtk_builder_get_object(builder_pref,"proxy_password"));
	gwget_pref.proxy_password=g_strdup(gtk_entry_get_text(GTK_ENTRY(proxy_password)));
	gconf_client_set_string(gconf_client,"/apps/gwget2/proxy_password",
							g_strdup(gtk_entry_get_text(GTK_ENTRY(proxy_password))),NULL);
	
	http_proxy_port_spin = GTK_WIDGET (gtk_builder_get_object (builder_pref, "http_proxy_port_spin"));
	gwget_pref.http_proxy_port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(http_proxy_port_spin));
	gconf_client_set_int(gconf_client,"/apps/gwget2/http_proxy_port",
						  gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(http_proxy_port_spin)),NULL);
						  
	proxy_uses_auth_radio=GTK_WIDGET (gtk_builder_get_object(builder_pref,"proxy_uses_auth_radio"));

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (proxy_uses_auth_radio))) {
		gconf_client_set_bool(gconf_client,"/apps/gwget2/network_mode",TRUE,NULL);
		gwget_pref.proxy_uses_auth=TRUE;
	}
	
	manual_radio=GTK_WIDGET (gtk_builder_get_object(builder_pref,"manual_radio"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(manual_radio))) {
		gconf_client_set_string(gconf_client,"/apps/gwget2/network_mode","manual",NULL);
		gwget_pref.network_mode="manual";
	}
			
	direct_radio=GTK_WIDGET (gtk_builder_get_object(builder_pref,"direct_radio"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(direct_radio))) {
		gconf_client_set_string(gconf_client,"/apps/gwget2/network_mode","direct",NULL);
		gwget_pref.network_mode="direct";
	}	
	
	default_radio=GTK_WIDGET (gtk_builder_get_object(builder_pref,"default_radio"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(default_radio))) {
		gconf_client_set_string(gconf_client,"/apps/gwget2/network_mode","default",NULL);
		gwget_pref.network_mode="default";
	}	
		
	ask_each_dl = GTK_WIDGET (gtk_builder_get_object(builder_pref, "ask_save_each_dl_check"));
	gwget_pref.ask_save_each_dl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ask_each_dl));
	gconf_client_set_bool(gconf_client, "/apps/gwget2/ask_save_each_dl",
				gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ask_each_dl)), NULL);
	
	
	num_retries = GTK_WIDGET (gtk_builder_get_object(builder_pref,"num_retries_spin"));
	gwget_pref.num_retries = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(num_retries));
	
	resume=GTK_WIDGET (gtk_builder_get_object(builder_pref,"resume_at_start"));
	gwget_pref.resume_at_start=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(resume));

	open_after_dl = GTK_WIDGET (gtk_builder_get_object(builder_pref, "open_after_dl"));
	gwget_pref.open_after_dl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(open_after_dl));
	gconf_client_set_bool(gconf_client, "/apps/gwget2/open_after_dl",
					gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(open_after_dl)), NULL);
	
	pref_window = GTK_WIDGET (gtk_builder_get_object(builder_pref,"pref_window"));
	gtk_widget_hide(pref_window);
	
	gconf_client_set_string(gconf_client,"/apps/gwget2/download_dir",
							g_strdup(gtk_entry_get_text(GTK_ENTRY(save_in))),NULL);
	gconf_client_set_int(gconf_client,"/apps/gwget2/num_retries",
						atoi(gtk_entry_get_text(GTK_ENTRY(num_retries))),NULL);
	gconf_client_set_bool(gconf_client,"/apps/gwget2/resume_at_start",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(resume)),NULL);
	
	/* Limit Speed */
	limit_speed_check = GTK_WIDGET (gtk_builder_get_object (builder_pref, "limit_speed_check"));
	limit_speed_spin = GTK_WIDGET (gtk_builder_get_object (builder_pref, "limit_speed_spin"));

	if ( (count_download_in_progress()>0) && 
	    ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(limit_speed_check)) &&
	    (gwget_pref.max_speed!=gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(limit_speed_spin)))) ||
	    ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(limit_speed_check)) && 
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(limit_speed_check))!=gwget_pref.limit_speed)))
	{
		inform_limit_speed_change();
	}	
	gwget_pref.limit_speed = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(limit_speed_check));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/limit_speed",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(limit_speed_check)),NULL);
	gwget_pref.max_speed = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(limit_speed_spin));
	gconf_client_set_int(gconf_client,"/apps/gwget2/max_speed",
						  gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(limit_speed_spin)),NULL);
	
	/* Limit Simultaneous downloads */
	limit_simultaneousdownloads_check = GTK_WIDGET (gtk_builder_get_object (builder_pref, "limit_simultaneousdownloads_check"));
	gwget_pref.limit_simultaneousdownloads = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(limit_simultaneousdownloads_check));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/limit_simultaneousdownloads",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(limit_simultaneousdownloads_check)),NULL);
	limit_simultaneousdownloads_spin = GTK_WIDGET (gtk_builder_get_object (builder_pref, "limit_simultaneousdownloads_spin"));
	gwget_pref.max_simultaneousdownloads = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(limit_simultaneousdownloads_spin));
	gconf_client_set_int(gconf_client,"/apps/gwget2/max_simultaneousdownloads",
						  gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(limit_simultaneousdownloads_spin)),NULL);
	
	/* Recursivity */
	no_create_directories  = GTK_WIDGET (gtk_builder_get_object(builder_pref,"no_create_directories"));
	gwget_pref.no_create_directories = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(no_create_directories));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/no_create_directories",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(no_create_directories)),NULL);
	
	/* Follow relative links only */
	follow_relative = GTK_WIDGET (gtk_builder_get_object(builder_pref,"follow_relative"));
	gwget_pref.follow_relative = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(follow_relative));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/follow_relative",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(follow_relative)), NULL);
	
	/* Convert links */
	convert_links = GTK_WIDGET (gtk_builder_get_object(builder_pref,"convert_links"));
	gwget_pref.follow_relative = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(convert_links));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/convert_links",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(convert_links)), NULL);
	/* Download page requisites */
	dl_page_requisites= GTK_WIDGET (gtk_builder_get_object(builder_pref,"dl_page_requisites"));
	gwget_pref.follow_relative = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dl_page_requisites));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/dl_page_requisites",	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dl_page_requisites)), NULL);
	/* Max Depth */
	max_depth=GTK_WIDGET (gtk_builder_get_object(builder_pref,"max_depth"));
	gwget_pref.max_depth=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(max_depth));
	gconf_client_set_int(gconf_client,"/apps/gwget2/max_depth",gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(max_depth)), NULL);
	
	/* Column listing */
	gwget_pref.view_actual_size=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_actual_size"))));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_actual_size",
	 					  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_actual_size")))),NULL);

	gwget_pref.view_total_size=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_total_size"))));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_total_size",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_total_size")))),NULL);
	
	gwget_pref.view_percentage=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_percentage"))));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_percentage",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_percentage")))),NULL);


	gwget_pref.view_elapse_time=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_elapse_time"))));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_elapse_time",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_elapse_time")))),NULL);

	gwget_pref.view_rem_time=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_rem_time"))));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_rem_time",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_rem_time")))),NULL);

	gwget_pref.view_down_speed=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_down_speed"))));
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_down_speed",
						  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_WIDGET (gtk_builder_get_object(builder_pref,"check_down_speed")))),NULL);

}

void
on_browse_save_in_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *filesel = NULL;
	GtkWidget *save_in;
	save_in = GTK_WIDGET (gtk_builder_get_object(builder_pref,"save_in_entry"));
	
	filesel = gtk_file_chooser_dialog_new  (_("Select Folder"),
						NULL,
						GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
						GTK_STOCK_CANCEL, 
						GTK_RESPONSE_CANCEL,
						GTK_STOCK_OPEN, 
						GTK_RESPONSE_ACCEPT,
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
		msg = g_strdup_printf(_("Really cancel current download?\n(URL: %s)"),gwgetdata->url);
		response = run_dialog(_("Delete download?"),_(msg), GTK_STOCK_CANCEL, _("Delete"));
		if (response==GTK_RESPONSE_OK) {
			gwget_data_stop_download(gwgetdata); 
			unlink (gwgetdata->local_filename);
    			gwgetdata->total_time = 0;
    			gwget_data_set_state (gwgetdata, DL_COMPLETED);
			if (gwgetdata == gwget_data_get_selected()) {
				gtk_window_set_title(GTK_WINDOW(GTK_WIDGET (gtk_builder_get_object(builder, "main_window"))), _("Gwget - Download Manager"));
			}
			gtk_list_store_remove(GTK_LIST_STORE(model),&gwgetdata->file_list);
			downloads=g_list_remove(downloads,gwgetdata);
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
	
	if (count_all_downloads()>0) 
	{
		response = run_dialog(_("Remove completed"),_("Really remove completed downloads from the list?"), GTK_STOCK_CANCEL, _("Remove"));
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
			gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(builder, "clear_button")), FALSE);
			gtk_window_set_title(GTK_WINDOW(GTK_WIDGET (gtk_builder_get_object(builder, "main_window"))), _("Gwget - Download Manager"));
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
	
	if (count_all_downloads()>0) 
	{
		response = run_dialog(_("Remove inactive"),_("Really remove inactive downloads from the list?"), GTK_STOCK_CANCEL, _("Remove inactive"));
		if (response == GTK_RESPONSE_OK) {
			length=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),NULL);
			gtk_tree_model_get_iter_root(model,&iter);
			for (i=0;i<length;i++) {
				gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
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
	
	if (count_all_downloads()>0) 
	{
		response = run_dialog(_("Remove all"),_("Really remove all downloads from the list?"), GTK_STOCK_CANCEL, _("Remove all"));
		if (response == GTK_RESPONSE_OK) {
			length=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),NULL);
			gtk_tree_model_get_iter_root(model,&iter);
			for (i=0;i<length;i++) {
				gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
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
			gtk_window_set_title(GTK_WINDOW(GTK_WIDGET (gtk_builder_get_object(builder, "main_window"))), _("Gwget - Download Manager"));
		}
	}

}

void 
on_view_toolbar_activate(GtkWidget *widget,gpointer data)
{
	GtkWidget *toolbar,*menu_item;
	gboolean state;
	
	toolbar=GTK_WIDGET (gtk_builder_get_object(builder,"toolbar1"));
	menu_item=GTK_WIDGET (gtk_builder_get_object(builder,"view_toolbar"));
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
on_view_statusbar_activate(GtkWidget *widget, gpointer data)
{
	GtkWidget *statusbar, *menu_item;
	gboolean state;

	statusbar = GTK_WIDGET (gtk_builder_get_object (builder, "statusbar"));
	menu_item=GTK_WIDGET (gtk_builder_get_object(builder,"view_statusbar"));
	state = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item));
	
	if (!state) 
	{
		gtk_widget_hide(GTK_WIDGET(statusbar));
	} else {
		gtk_widget_show(GTK_WIDGET(statusbar));
	}
	gconf_client_set_bool(gconf_client,"/apps/gwget2/view_statusbar",state,NULL);
}

void 
on_properties_activate(GtkWidget *widget, gpointer data)
{
	GtkWidget *main_window,*properties,*url_txt,*local_file,*local_dir;
	GwgetData *gwgetdata;

	gwgetdata = gwget_data_get_selected();	
	
	if (gwgetdata) {
		main_window=GTK_WIDGET (gtk_builder_get_object(builder,"main_window"));
		properties=GTK_WIDGET (gtk_builder_get_object(builder,"properties_window"));
		gtk_window_set_transient_for(GTK_WINDOW(properties),GTK_WINDOW(main_window));
		url_txt=GTK_WIDGET (gtk_builder_get_object(builder,"url_text"));
		gtk_label_set_text(GTK_LABEL(url_txt),gwgetdata->url);
		local_file=GTK_WIDGET (gtk_builder_get_object(builder,"local_file_text"));
		gtk_label_set_text(GTK_LABEL(local_file),gwgetdata->filename);
		local_dir=GTK_WIDGET (gtk_builder_get_object(builder,"local_dir"));
		gtk_label_set_text(GTK_LABEL(local_dir),gwgetdata->dir);
		if (gwgetdata->state==DL_COMPLETED) {
                   gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(builder, "compare_md5")), TRUE);
		} else {
                   gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(builder, "compare_md5")), FALSE);
                }
                gtk_widget_show(properties);
	}
}

void
on_compare_md5_clicked(GtkWidget *widget, gpointer data)
{
   GwgetData *gwgetdata;

   gwgetdata=gwget_data_get_selected();
	
   if (gwgetdata) {
      gtk_widget_show(GTK_WIDGET (gtk_builder_get_object(builder,"md5_window")));
   }
}

void
on_md5ok_button_clicked(GtkWidget *widget, gpointer data)
{
   GtkWidget *md5;
   GwgetData *gwgetdata;
   const gchar *entrytext;
   GtkWidget *dialog;
   GtkMessageType msg_type;

   GString *md5res;
   GString *msg;
   gint i;
   
   struct md5_ctx md5context;
   guchar md5digest[16];
   gchar dataread[MD5BUFSIZE];
   gboolean close_md5_dlg;
   GFile *file;
   GInputStream *stream;
   gssize *sizeread;
  
   gwgetdata = gwget_data_get_selected();
	
   if (gwgetdata) {
      md5 = GTK_WIDGET (gtk_builder_get_object(builder,"md5_window"));
      
      file = g_file_new_for_path (gwgetdata->local_filename);
      
      stream = (GInputStream *) g_file_read (file, NULL, NULL);
      sizeread = (gssize *)g_input_stream_read (stream, dataread, MD5BUFSIZE, NULL,NULL);
      
      md5_init_ctx(&md5context);

      while ( sizeread>0 ) {
				md5_process_bytes (dataread, (int)sizeread, &md5context);
				sizeread = (gssize *)g_input_stream_read (stream, dataread, MD5BUFSIZE, NULL,NULL);
      }

      entrytext = gtk_entry_get_text(GTK_ENTRY (gtk_builder_get_object(builder, "md5_entry")));

      md5_finish_ctx(&md5context, md5digest);

			md5res=g_string_new("");
      for (i = 0; i < 16; i++) {
         g_string_append_printf(md5res,"%02x", md5digest[i]);
      }

      msg = g_string_new("<span weight=\"bold\">");
      
      if (g_str_equal(md5res->str, entrytext)) {
         close_md5_dlg=TRUE;
         g_string_append_printf(msg, _("MD5SUM Check PASSED!"));
         msg_type=GTK_MESSAGE_INFO;
      } else {
         g_string_append_printf(msg, _("MD5SUM Check FAILED!"));
         close_md5_dlg=FALSE;
         msg_type=GTK_MESSAGE_WARNING;
      }
      g_string_append_printf(msg, "</span>");

      dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(md5),
                                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                                     msg_type,
                                                     GTK_BUTTONS_CLOSE,
                                                     NULL);

      gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG(dialog), msg->str);
                                  
      gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(GTK_WIDGET(dialog));   
      g_string_free(md5res, TRUE);
      g_string_free(msg, TRUE);
      g_object_unref (stream);
      g_object_unref (file);

      if (close_md5_dlg) {
         gtk_widget_hide(GTK_WIDGET(md5));
      }
   }
}

void 
on_check_actual_size_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = GTK_WIDGET (gtk_builder_get_object(builder,"treeview1"));
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),CURRENTSIZE_COLUMN-2);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
}

void 
on_check_total_size_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = GTK_WIDGET (gtk_builder_get_object(builder, "treeview1"));
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),TOTALSIZE_COLUMN-2);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
	
}

void 
on_check_percentage_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = GTK_WIDGET (gtk_builder_get_object(builder,"treeview1"));
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),PERCENTAGE_COLUMN-2);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
	
}

void 
on_check_elapse_time_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = GTK_WIDGET (gtk_builder_get_object(builder,"treeview1"));
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),ELAPSETIME_COLUMN-3);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
	
}

void 
on_check_rem_time_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = GTK_WIDGET (gtk_builder_get_object(builder,"treeview1"));
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),REMAINTIME_COLUMN-5);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
	
}

void 
on_check_down_speed_toggled(GtkWidget *widget, gpointer data)
{
	GtkWidget *treev,*column;
	gboolean visible;
	
	treev = GTK_WIDGET (gtk_builder_get_object(builder,"treeview1"));
	column=(GtkWidget *)gtk_tree_view_get_column(GTK_TREE_VIEW(treev),SPEED_COLUMN-7);
	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_tree_view_column_set_visible(GTK_TREE_VIEW_COLUMN(column),
                                     visible);
	
}

void
on_limit_speed_check_toggled (GtkWidget *widget, gpointer data)
{
	GtkWidget *limit_speed_spin;
	gboolean limit_speed;
	
	limit_speed_spin = GTK_WIDGET (gtk_builder_get_object (builder_pref, "limit_speed_spin"));
	limit_speed = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget));
	
	if (limit_speed) {
		gtk_widget_set_sensitive (GTK_WIDGET(limit_speed_spin), TRUE);
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET(limit_speed_spin), FALSE);
	}
}

void
on_limit_simultaneousdownloads_check_toggled (GtkWidget *widget, gpointer data)
{
	GtkWidget *limit_simultaneousdownloads_spin;
	gboolean limit_simultaneousdownloads;
	
	limit_simultaneousdownloads_spin = GTK_WIDGET (gtk_builder_get_object (builder_pref, "limit_simultaneousdownloads_spin"));
	limit_simultaneousdownloads = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget));
	
	if (limit_simultaneousdownloads) {
		gtk_widget_set_sensitive (GTK_WIDGET(limit_simultaneousdownloads_spin), TRUE);
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET(limit_simultaneousdownloads_spin), FALSE);
	}
}
void 
on_manual_radio_toggled (GtkWidget *sender, gpointer data) 
{
	GtkWidget *widget;

	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref,"http_proxy_entry"));
	gtk_widget_set_sensitive(GTK_WIDGET (widget), TRUE);

	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "http_proxy_port_spin"));
	gtk_widget_set_sensitive(GTK_WIDGET (widget), TRUE);

	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "proxy_uses_auth_radio"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), TRUE);
	
	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "proxy_user"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), TRUE);
	
	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "proxy_password"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), TRUE);

}

void 
on_direct_radio_toggled (GtkWidget *sender, gpointer data) 
{
	GtkWidget *widget;

	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref,"http_proxy_entry"));
	gtk_widget_set_sensitive(GTK_WIDGET (widget), FALSE);
	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "http_proxy_port_spin"));
	gtk_widget_set_sensitive(GTK_WIDGET (widget), FALSE);

	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "http_proxy_port_spin"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);
	
	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "proxy_uses_auth_radio"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);
	
	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "proxy_user"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);
	
	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "proxy_password"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);

}

void 
on_default_radio_toggled(GtkWidget *sender, gpointer data) 
{	
	GtkWidget *widget;

	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "http_proxy_entry"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);
	
	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "http_proxy_port_spin"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);
	
	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "proxy_uses_auth_radio"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), TRUE);
	
	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "proxy_user"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), TRUE);
	
	widget = GTK_WIDGET (gtk_builder_get_object(builder_pref, "proxy_password"));
	gtk_widget_set_sensitive(GTK_WIDGET(widget), TRUE);
}

void
check_download_in_progress(void)
{
	GwgetData* gwgetdata;
	GtkTreeIter iter;
	gint length,i;
	gchar *url;
	gboolean inprogress;

	length = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), NULL);
	gtk_tree_model_get_iter_root(model, &iter);

	inprogress = FALSE;
	for (i=0;i<length;i++) {
		gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
		gwgetdata=g_object_get_data(G_OBJECT(model), url);
			
		if (gwgetdata->state==DL_RETRIEVING) {
			inprogress = TRUE;
			break;
		}
		gtk_tree_model_iter_next(model,&iter);
	}

	if (inprogress) {
		set_icon_downloading();
	} else {
		set_icon_idle();
	}
}

void 
start_first_waiting_download(void)
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
			
		if (gwgetdata->state==DL_WAITING) {
			gwget_data_start_download(gwgetdata);
			return;
		}
	gtk_tree_model_iter_next(model,&iter);
	}
}

/* Count active downloads */
gint count_download_in_progress(void) {
	GwgetData* gwgetdata;
	GtkTreeIter iter;
	gint number;
	gchar *url;
	gboolean iter_valid;

	number=0;
	for (iter_valid = gtk_tree_model_get_iter_first (model, &iter);iter_valid;iter_valid = gtk_tree_model_iter_next (model, &iter)) {
		gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
		gwgetdata=g_object_get_data(G_OBJECT(model), url);
		if ((gwgetdata->state != DL_NOT_STARTED) && 
		    (gwgetdata->state != DL_WAITING) && 
		    (gwgetdata->state != DL_NOT_RUNNING) && 
		    (gwgetdata->state != DL_COMPLETED)) {
			number++;
		}
	}
	return number;
}

/* Count all downloads */
gint
count_all_downloads(void)
{
	GtkTreeIter iter;
	gint number;
	gboolean iter_valid;

	number=0;
	for (iter_valid = gtk_tree_model_get_iter_first (model, &iter);iter_valid;iter_valid = gtk_tree_model_iter_next (model, &iter)) {
		number++;
	}
	return number;
}


void
on_download_menu_activate(void) 
{
	gwget_data_set_menus (gwget_data_get_selected());
}

void
on_edit_menu_activate(void)
{
	if (count_all_downloads()>0) 
	{
		 gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(builder, "remove_item")), TRUE);
	} else {
		 gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(builder, "remove_item")), FALSE);
	}
}

static
void inform_limit_speed_change(void)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (NULL,
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK,
					_("New download speed limit will apply only to new or restarted downloads"));
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

void
on_remove_download_activate (GtkWidget *widget, gpointer data)
{
	GwgetData *gwgetdata;
	GtkTreeIter iter;
	gchar *url;
	gchar *message;
	gint response, length, i;

	gwgetdata = gwget_data_get_selected();
	if (gwgetdata != NULL) {
	    message = g_strdup_printf (_("Remove %s ?"), gwgetdata->filename);
		response = run_dialog (message, _("Really remove this download from the list?"), GTK_STOCK_CANCEL, _("Remove download"));
		if (response == GTK_RESPONSE_OK) {
			length=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),NULL);
			gtk_tree_model_get_iter_root(model,&iter);
			for (i=0;i<length;i++) {
				gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
				if (gwgetdata==g_object_get_data(G_OBJECT(model),url)) {
					gtk_list_store_remove(GTK_LIST_STORE(model),&iter);
					downloads=g_list_remove(downloads,gwgetdata);
				} else {
					gtk_tree_model_iter_next(model,&iter);
				}
			}
		}
	}
}

void
on_open_download_activate (GtkWidget *widget, gpointer data)
{
	GwgetData *gwgetdata;
	gchar *uri;
	GFile *location;
	GError *err = NULL;

	gwgetdata=gwget_data_get_selected();
	g_return_if_fail (gwgetdata!=NULL);

	location = g_file_parse_name (gwgetdata->local_filename);

	uri = g_file_get_uri (location);
	
	if (!gtk_show_uri (NULL, uri, GDK_CURRENT_TIME, &err)) {
		run_dialog_error (_("Error opening file"),_("Couldn't open the file"));
		return;
	}

	g_object_unref (location);
}

void
on_open_directory_activate (GtkWidget *widget, gpointer data)
{
	GwgetData *gwgetdata;
	gchar *uri;
	GFile *location;
	GError *err = NULL;

	gwgetdata=gwget_data_get_selected();
	g_return_if_fail (gwgetdata!=NULL);

	location = g_file_parse_name (gwgetdata->dir);

	uri = g_file_get_uri (location);

	if (!gtk_show_uri (NULL, uri, GDK_CURRENT_TIME, &err)) {
		run_dialog_error (_("Error opening file"),_("Couldn't open the folder"));
		return;
	}

	g_object_unref (location);
}

void
on_file_menuitem_activate (GtkWidget *widget, gpointer data)
{

	if (count_all_downloads()==0)
	{
		gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(builder,"pause_all_menuitem")), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(builder,"resume_all_menuitem")), FALSE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(builder,"pause_all_menuitem")), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET (gtk_builder_get_object(builder,"resume_all_menuitem")), TRUE);
	}

}
