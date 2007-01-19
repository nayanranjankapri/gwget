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
 

#ifndef _NEW_WINDOW_H
#define _NEW_WINDOW_H


void on_ok_button_clicked(GtkWidget *widget, gpointer data);
int check_url(char *str1, char *str2);
void create_new_window(void);
void on_cancel_button_clicked(GtkWidget *widget,gpointer data);

/* Save in Fileselection callbacks */
void on_new_browse_save_in_button_clicked(GtkWidget *widget, gpointer data);
void on_new_fs_ok_button_clicked(GtkWidget *widget, gpointer data);
void on_new_fs_cancel_button_clicked(GtkWidget *widget, gpointer data);
void create_new_window_with_url (gchar *url);

#endif
