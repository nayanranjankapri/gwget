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

#include <gnome.h>
#include "main_window.h"
#include "utils.h"
#include "gwget_data.h"



gint 
run_dialog (gchar *title, gchar *message, gchar *cancel_message, gchar *action_message)
{
	GtkWidget *dialog;
	gint response;
	
	dialog = gtk_message_dialog_new_with_markup ( GTK_WINDOW (GTK_WIDGET (gtk_builder_get_object(builder, "main_window")) ),
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
			"<span weight='bold' size='large'>%s</span>",
			title);

	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", message);
		/* message may contain '%' characters, so we must not use it
		 * as the format string directly. */
	
	gtk_dialog_add_buttons (GTK_DIALOG(dialog), cancel_message, GTK_RESPONSE_CANCEL,
			action_message, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	gtk_window_set_title (GTK_WINDOW (dialog), "");
	response=gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(GTK_WIDGET(dialog));
	return response;
}

void 
run_dialog_information(gchar *title, gchar *msg)
{
	GtkWidget *dialog;

  dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (GTK_WIDGET(gtk_builder_get_object (builder, "main_window"))),
          GTK_DIALOG_MODAL,
          GTK_MESSAGE_INFO,
          GTK_BUTTONS_CLOSE,
          "%s", msg);
          
  gtk_window_set_title (GTK_WINDOW (dialog), title);
  
  g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
  
  gtk_widget_show (dialog);
}

void
run_dialog_error (gchar *title, gchar *message)
{
    GtkWidget *dialog;
    
    dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (GTK_WIDGET (gtk_builder_get_object(builder, "main_window"))),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_CLOSE,
            "%s", message);
            
    gtk_window_set_title (GTK_WINDOW (dialog), title);
    
    g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
    
    gtk_widget_show (dialog);
}

int check_url( char *str1, char *str2 ) 
{
	int i;
	for( i = 0; str1[i] != '\0'; i++ ) 
	{
		if( str1[i] != str2[i] ) return(0);
	}
	return(1);
}

gboolean check_url_already_exists(gchar *checkurl)
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
		if (!strcmp(url, checkurl)) {
			return TRUE;
		}
		gtk_tree_model_iter_next(model,&iter);
	}
	return FALSE;
}

gboolean check_server_already_exists(gchar *checkurl)
{
	GwgetData* gwgetdata;
	GtkTreeIter iter;
	gint length,i;
	gchar *url, *ptr, *ptrb;

	/* extract server name from url */
	ptr = strchr (checkurl, ':');
	if (ptr != NULL) {
		ptr += 3;
		ptrb = strchr (ptr, '/');
		if (ptrb != NULL) {
			checkurl = g_strndup (ptr, ptrb-ptr);
		} else {
			checkurl = g_strdup (ptr);
		}
	}

	length=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model),NULL);
	gtk_tree_model_get_iter_root(model,&iter);
	for (i=0;i<length;i++) {
		gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
		gwgetdata=g_object_get_data(G_OBJECT(model),url);

		ptr = strchr (url, ':');
		if (ptr != NULL) {
			ptr += 3;
			ptrb = strchr (ptr, '/');
			if (ptrb != NULL) {
				url = g_strndup (ptr, ptrb-ptr);
			} else {
				url = g_strdup (ptr);
			}
		}

		if (strcmp(url, checkurl)) {
			return TRUE;
		}
		gtk_tree_model_iter_next(model,&iter);
	}
	return FALSE;
}

gchar* get_readable_size(guint64 size_in_kb)
{
  float n_mb;
  gchar *size;

  if (size_in_kb == 0)
    size = g_strdup("0");
  else
  {
    n_mb = (float)((guint32)(size_in_kb  + 512)/1048576);

    if(n_mb >= 1024)
	    size = g_strdup_printf ("%0.2f Gb", (float)(guint32)(size_in_kb  + 512) / 1073741824);
    else
      if (n_mb >= 1)
          size = g_strdup_printf ("%0.2f Mb", (float)(guint32)(size_in_kb  + 512) / 1048576);
      else
          size = g_strdup_printf ("%0.2f Kb", (float)(guint32)(size_in_kb  + 512) / 1024);
  }

  return size;
}

