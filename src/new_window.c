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


static void add_to_save_in_combobox (gpointer data1, gpointer data2);

/* xml of the new download window */
GladeXML *xml_new = NULL;

void on_ok_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *window=NULL;
	GtkEntry *url_entry=NULL,*save_in_entry=NULL;
	gchar *url=NULL,*save_in=NULL;
	GwgetData *gwgetdata;
		
	window = glade_xml_get_widget(xml_new,"new_window");
	url_entry = GTK_ENTRY(glade_xml_get_widget(xml_new,"url_entry"));
	save_in_entry=GTK_ENTRY(glade_xml_get_widget(xml_new,"save_in_entry"));
	
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
	
	window = glade_xml_get_widget(xml_new,"new_window");
	
	gtk_widget_hide(window);
}

/* checks for data in clipboard: URL or not */
int check_url( char *str1, char *str2 ) 
{
	int i;
	for( i = 0; str1[i] != '\0'; i++ ) 
	{
		if( str1[i] != str2[i] ) return(0);
	}
	return(1);
}

void 
create_new_window(void)
{
	gchar *xml_file = NULL;
	GtkWidget *window = NULL, *combo;
	GtkEntry *entry = NULL;
	gchar *url = ""; // URL in clipboard
	GtkClipboard *clipboard = NULL;

	clipboard = gtk_clipboard_get (GDK_NONE);
	if (clipboard!=NULL) {
		url = gtk_clipboard_wait_for_text (clipboard);
	}
		
	if (!xml_new) {
		xml_file =g_build_filename(DATADIR,"newdownload.glade",NULL);
		xml_new = glade_xml_new(xml_file,NULL,NULL);
		glade_xml_signal_autoconnect(xml_new);
	}
	
	window = glade_xml_get_widget(xml_new,"new_window");

	/* if clipboards data is an URL, then leave url value as is, else -- empty string */
	entry = GTK_ENTRY(glade_xml_get_widget(xml_new,"url_entry"));
	if ( (url!=NULL) && !check_url( "http://", url ) && !check_url( "ftp://", url))
		url = "";
	
	gtk_entry_set_text(GTK_ENTRY(entry),url);

	combo = glade_xml_get_widget (xml_new, "save_in_comboboxentry");
	gtk_combo_box_set_model(GTK_COMBO_BOX(combo), save_in_model);
	gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY(combo), 0);
	
	g_list_foreach (save_in_paths, add_to_save_in_combobox, NULL);
	
	gtk_widget_show(window);
}

void
on_new_browse_save_in_button_clicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *filesel,*save_in_entry;
	
	filesel = gtk_file_chooser_dialog_new  (_("Select Folder"),
						NULL,
						GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						NULL);
	
	save_in_entry = glade_xml_get_widget(xml_new,"save_in_entry");
	if (gtk_dialog_run (GTK_DIALOG (filesel)) == GTK_RESPONSE_ACCEPT) {
		char *directory;
		
		directory = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
		gtk_entry_set_text(GTK_ENTRY(save_in_entry),directory);
		
		g_free(directory);
	}
	
	gtk_widget_destroy(filesel);

}

static void
add_to_save_in_combobox (gpointer data1, gpointer data2)
{
	gchar *option = data1;
	GtkTreeIter iter;
		
	printf("Op: %s\n", option);
	gtk_list_store_append (GTK_LIST_STORE(save_in_model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(save_in_model), &iter, 0, option, -1);
	
}
