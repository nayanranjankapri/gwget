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



gint 
run_dialog(gchar *title, gchar *msg)
{
	GtkWidget *dialog;
	gchar *mark;
	gint response;
	
	dialog = glade_xml_get_widget(xml,"dialog1");
	mark=g_strdup_printf("<span size=\"large\">%s</span>",title);
	gtk_label_set_markup(GTK_LABEL(glade_xml_get_widget(xml,"title_label")),mark);
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml,"msg_label")),msg);
	
	response=gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(GTK_WIDGET(dialog));
	return response;
}
