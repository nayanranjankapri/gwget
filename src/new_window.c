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
 


/* 

This file creates the window for add new download and its callback


*/


#include <gnome.h>
#include <glade/glade.h>
#include <config.h>
#include "new_window.h"
#include "main_window.h"
#include "main_window_cb.h"
#include "gwget_data.h"
#include "utils.h"


/* xml of the new download window */
GladeXML *xml_new = NULL;


	

void on_ok_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *window=NULL,*recursive_window=NULL,*radio=NULL;
	GtkEntry *url_entry=NULL,*save_in_entry=NULL;
	gchar *url=NULL,*save_in=NULL;
	GwgetData *gwgetdata;
	gint response;
	
	
	window = glade_xml_get_widget(xml_new,"new_window");
	url_entry = GTK_ENTRY(glade_xml_get_widget(xml_new,"url_entry"));
	save_in_entry=GTK_ENTRY(glade_xml_get_widget(xml_new,"save_in_entry"));
	gtk_widget_hide(window);
	
	url=  (gchar *)(gtk_entry_get_text(GTK_ENTRY(url_entry)));
	
	if (strcmp(url,"")) {
		url = g_strdup(url);
		save_in=g_strdup(gtk_entry_get_text(GTK_ENTRY(save_in_entry)));
	
		if (!strcmp(save_in,"") && gwget_pref.download_dir) {
			save_in=g_strdup(gwget_pref.download_dir);
		}
	
		if (!strcmp(save_in,"") && !gwget_pref.download_dir) {
		    save_in=g_strdup(getenv("HOME"));
		}
	
		gwgetdata = gwget_data_create(url,save_in);
		/* if the url it's not a file drop a dialog to recurse into the url */
		if (!strcmp(gwgetdata->filename,"") || !strcmp(gwgetdata->filename,gwgetdata->url)) {
			/* response=run_dialog(_("Recurse into the Url ?"),_("The url it's not a file. \n Do you like to recurse into the url ?\nOtherwise only the index file will be downloaded."));
			if (response == GTK_RESPONSE_OK) {
				gwgetdata->recursive=TRUE;
			}
			*/
			recursive_window=glade_xml_get_widget(xml,"dialog2");
			response=gtk_dialog_run(GTK_DIALOG(recursive_window));
			gtk_widget_hide(GTK_WIDGET(recursive_window));
			if (response==GTK_RESPONSE_OK) {
				radio=glade_xml_get_widget(xml,"radio_index");
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
					gwgetdata->recursive=FALSE;
				}
				radio=glade_xml_get_widget(xml,"radio_multimedia");
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
					printf("Multimedia\n");
					gwgetdata->recursive=TRUE;
					gwgetdata->multimedia=TRUE;
				}
				radio=glade_xml_get_widget(xml,"radio_mirror");
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
					printf("mirror\n");
					gwgetdata->recursive=TRUE;
					gwgetdata->mirror=TRUE;
				}
				radio=glade_xml_get_widget(xml,"radio_recursive");
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
					gwgetdata->recursive=TRUE;
				}
			}
		}
		downloads = g_list_append(downloads,gwgetdata);
		gwget_data_set_state(gwgetdata,DL_NOT_CONNECTED);
		new_download(gwgetdata);
		gwget_data_start_download(gwgetdata);
		g_free(save_in);		
	}
	
	
	
}

void on_cancel_button_clicked(GtkWidget *widget,gpointer data) 
{
	GtkWidget *window = NULL;
	
	window = glade_xml_get_widget(xml_new,"new_window");
	
	gtk_widget_hide(window);
}
	

void create_new_window(void)
{
	gchar *xml_file = NULL;
	GtkWidget *window = NULL;
	GtkEntry *entry = NULL;
	
	if (!xml_new) {
		xml_file =g_build_filename(DATADIR,"newdownload.glade",NULL);
		xml_new = glade_xml_new(xml_file,NULL,NULL);
		glade_xml_signal_autoconnect(xml_new);
	}
	
	window = glade_xml_get_widget(xml_new,"new_window");
	entry = GTK_ENTRY(glade_xml_get_widget(xml_new,"url_entry"));
	gtk_entry_set_text(GTK_ENTRY(entry),"");
	entry = GTK_ENTRY(glade_xml_get_widget(xml_new,"save_in_entry"));
	if (!gwget_pref.download_dir) 
	{
		gtk_entry_set_text(GTK_ENTRY(entry),g_get_home_dir());
	}
	else 
	{
		gtk_entry_set_text(GTK_ENTRY(entry),gwget_pref.download_dir);
	}
	gtk_widget_show(window);
}

void
on_new_browse_save_in_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *fs=NULL;
	
	fs=glade_xml_get_widget(xml_new,"new_fileselection1");
	gtk_widget_set_sensitive (GTK_WIDGET(GTK_FILE_SELECTION(fs)->file_list),FALSE);
	gtk_widget_show(fs);
	
}

void
on_new_fs_cancel_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *fs=NULL;
	
	fs=glade_xml_get_widget(xml_new,"new_fileselection1");
	gtk_widget_hide(fs);
}


void
on_new_fs_ok_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *fs=NULL,*save_in_entry=NULL;
	
	fs=glade_xml_get_widget(xml_new,"new_fileselection1");
	
	save_in_entry = glade_xml_get_widget(xml_new,"save_in_entry");
	
	gtk_entry_set_text(GTK_ENTRY(save_in_entry),gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)));
		
	gtk_widget_hide(fs);
}
