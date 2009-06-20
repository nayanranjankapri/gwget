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
#include <config.h>
#include "new_window.h"
#include "main_window.h"
#include "main_window_cb.h"
#include "gwget_data.h"
#include "utils.h"


static void add_to_save_in_combobox (gpointer data1, gpointer data2);


void on_ok_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *window=NULL, *combo;
	GtkEntry *save_in_entry=NULL;
	gchar *url=NULL,*save_in;
	gchar *save_in_list;
	GwgetData *gwgetdata;
		
	window = GTK_WIDGET (gtk_builder_get_object(builder, "new_window"));
	combo = GTK_WIDGET (gtk_builder_get_object (builder, "save_in_comboboxentry"));
	save_in_entry=GTK_ENTRY(GTK_BIN(combo)->child);
	
	url=g_strstrip((gchar *)(gtk_entry_get_text (GTK_ENTRY(GTK_WIDGET (gtk_builder_get_object(builder, "url_entry"))))));
	
	if (strcmp(url,"")) {
		url = g_strdup(url);
		save_in=g_strdup(gtk_combo_box_get_active_text (GTK_COMBO_BOX( GTK_WIDGET (gtk_builder_get_object(builder, "save_in_comboboxentry")))));
		
		if (!strcmp(save_in,"") && gwget_pref.download_dir) {
			save_in=g_strdup(gwget_pref.download_dir);
		}
	
		if (!strcmp(save_in,"") && !gwget_pref.download_dir) {
			save_in=g_strdup(g_get_home_dir());
		}
		
		save_in_list = g_strdup(save_in);
		if (g_list_find_custom(save_in_paths, save_in, (GCompareFunc) strcmp)==NULL) {
			save_in_paths = g_list_prepend (save_in_paths, save_in_list);
		}
		gwgetdata = gwget_data_create (url, save_in);
		gwget_data_add_download(gwgetdata);
		gwget_data_start_download(gwgetdata);
		gtk_widget_hide(window);
		g_free(save_in);
	}
}

void 
on_cancel_button_clicked(GtkWidget *widget,gpointer data) 
{
	GtkWidget *window = NULL;
	
	window = GTK_WIDGET (gtk_builder_get_object (builder,"new_window"));
	
	gtk_widget_hide (window);
}

void 
create_new_window(void)
{
	GtkWidget *window = NULL;
	GtkEntry *entry = NULL;
	gchar *url = NULL; // URL in clipboard
	GtkClipboard *clipboard = NULL;

	clipboard = gtk_clipboard_get (GDK_NONE);
	if (clipboard!=NULL) {
		url = g_strstrip(gtk_clipboard_wait_for_text (clipboard));
	}
		
	
	window = GTK_WIDGET (gtk_builder_get_object (builder, "new_window"));

	/* if clipboards data is an URL, then leave url value as is, else -- empty string */
	entry = GTK_ENTRY(GTK_WIDGET (gtk_builder_get_object (builder, "url_entry")));
	if ( (url!=NULL) && !check_url( "http://", url ) && !check_url( "ftp://", url)) {
		g_free(url);
		url = NULL;
	}
	
	if (url != NULL) {	
		gtk_entry_set_text(GTK_ENTRY(entry),url);
		g_free(url);
	}
	
	gtk_list_store_clear (GTK_LIST_STORE(save_in_model));
	g_list_foreach (save_in_paths, add_to_save_in_combobox, NULL);
	
	gtk_widget_show(window);
}

void
on_new_browse_save_in_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *filesel, *combo;
	GtkEntry *save_in_entry;
	
	filesel = gtk_file_chooser_dialog_new  (_("Select Folder"),
						NULL,
						GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						NULL);
	
	combo = GTK_WIDGET (gtk_builder_get_object (builder, "save_in_comboboxentry"));
	save_in_entry=GTK_ENTRY(GTK_BIN(combo)->child);
	
	if (gtk_dialog_run (GTK_DIALOG (filesel)) == GTK_RESPONSE_ACCEPT) {
		char *directory;
		
		directory = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
		gtk_entry_set_text(GTK_ENTRY(save_in_entry),directory);
		
		g_free(directory);
	}
	
	gtk_widget_destroy(filesel);

}

void 
create_new_window_with_url (gchar *url) 
{
	GtkWidget *window;
	GtkWidget *entry;
	
	window = GTK_WIDGET (gtk_builder_get_object (builder, "new_window"));
	entry = GTK_WIDGET (gtk_builder_get_object (builder, "url_entry"));
	gtk_entry_set_text (GTK_ENTRY(entry), url);
	
	gtk_list_store_clear (GTK_LIST_STORE(save_in_model));
	g_list_foreach (save_in_paths, add_to_save_in_combobox, NULL);
	
	gtk_widget_show (window);

}

static void
add_to_save_in_combobox (gpointer data1, gpointer data2)
{
	gchar *option = data1;
	GtkTreeIter iter;
		
	gtk_list_store_append (GTK_LIST_STORE(save_in_model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(save_in_model), &iter, 0, option, -1);
	
}
