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
#include <glade/glade.h>
#include "main_window.h"
#include "utils.h"
#include "gwget_data.h"



gint 
run_dialog(gchar *title, gchar *msg)
{
	GtkWidget *dialog;
	gchar *mark;
	gint response;
	
	dialog = glade_xml_get_widget(xml,"dialog1");
	mark=g_strdup_printf("<span size=\"large\" weight=\"bold\">%s</span>",title);
	gtk_label_set_markup(GTK_LABEL(glade_xml_get_widget(xml,"title_label")),mark);
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml,"msg_label")),msg);
	
	response=gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(GTK_WIDGET(dialog));
	return response;
}

gint 
run_dialog_information(gchar *title, gchar *msg)
{
	GtkWidget *dialog;
	gchar *mark;
	gint response;
	
	dialog = glade_xml_get_widget(xml,"dialog3");
	mark=g_strdup_printf("<span size=\"large\" weight=\"bold\">%s</span>",title);
	gtk_label_set_markup(GTK_LABEL(glade_xml_get_widget(xml,"title_label_inf")),mark);
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml,"msg_label_inf")),msg);
	
	response=gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(GTK_WIDGET(dialog));
	return response;
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

