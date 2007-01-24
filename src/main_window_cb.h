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
 

#ifndef _MAIN_WINDOW_CB_H
#define _MAIN_WINDOW_CB_H
#include "gwget_data.h"

void on_quit1_activate(GtkWidget *widget, gpointer data);
void on_about1_activate(GtkWidget *widget, gpointer data);

void on_button_new_clicked(GtkWidget *widget, gpointer data);
void on_cancel_button_clicked(GtkWidget *widget,gpointer data);
void on_about1_close(GtkWidget *widget, gpointer data);
void new_download(GwgetData *gwgetdata);
/* Preferences callbacks */
void on_pref_cancel_button_clicked(GtkWidget *widget,gpointer data);
void on_pref_ok_button_clicked(GtkWidget *widget,gpointer data);
void on_fs_cancel_button_clicked(GtkWidget *widget, gpointer data);
void on_fs_ok_button_clicked(GtkWidget *widget, gpointer data);
void on_boton_pref_clicked(GtkWidget *widget, gpointer data);
void on_browse_save_in_button_clicked(GtkWidget *widget, gpointer data);
void stop_all_downloads(void);
/* Popup menu in treeview callbacks */
gboolean on_treeview1_button_press_event(GtkWidget *widget, GdkEventButton *event,gpointer user_data);

/* Stop toolbar button callback */
void on_stop_button_clicked (GtkWidget *widget, gpointer data);
/* Selection Change in treeview callback ( Not yet implemented ) */
void tree_selection_changed_cb(GtkTreeSelection *selection, gpointer data);

/* Popup options */
void on_popup_pause_button_clicked(GtkWidget *widget, gpointer data);
void on_popup_continue_activate(GtkWidget *widget, gpointer data);
void on_cancel_download_activate(GtkWidget *widget,gpointer data);
void on_remove_completed_activate(GtkWidget *widget, gpointer data);
void on_remove_notrunning_activate(GtkWidget *widget, gpointer data);
void on_remove_all_activate(GtkWidget *widget, gpointer data);
void on_view_toolbar_activate(GtkWidget *widget,gpointer data);
void on_view_statusbar_activate(GtkWidget *widget,gpointer data);
void on_properties_activate(GtkWidget *widget, gpointer data);
/* CAllbacks for the column list view */
void on_check_file_type_toggled(GtkWidget *widget, gpointer data);
void on_check_actual_size_toggled(GtkWidget *widget, gpointer data);
void on_check_total_size_toggled(GtkWidget *widget, gpointer data);
void on_check_percentage_toggled(GtkWidget *widget, gpointer data);
void on_check_elapse_time_toggled(GtkWidget *widget, gpointer data);
void on_check_rem_time_toggled(GtkWidget *widget, gpointer data);
void on_check_down_speed_toggled(GtkWidget *widget, gpointer data);
void on_manual_radio_toggled (GtkWidget *widget, gpointer data);
void on_direct_radio_toggled (GtkWidget *widget, gpointer data);
void on_default_radio_toggled (GtkWidget *widget, gpointer data);

//Callback for md5_window
void on_md5ok_button_clicked(GtkWidget *widget, gpointer data); 

//Callback for properties window
void on_compare_md5_clicked(GtkWidget *widget, gpointer data);

/* Preferences */
void on_limit_speed_check_toggled (GtkWidget *widget, gpointer data);
void on_limit_simultaneousdownloads_check_toggled (GtkWidget *widget, gpointer data);

void check_download_in_progress(void);
void continue_all_downloads(void);
void new_download(GwgetData* gwgetdata);
void check_download_in_progress(void);
void start_first_waiting_download(void);
gint count_download_in_progress(void);
void on_download_menu_activate(void);
void on_edit_menu_activate(void);
void on_open_download_activate(GtkWidget *widget, gpointer data);
void on_remove_download_activate(GtkWidget *widget, gpointer data);
void on_open_directory_activate (GtkWidget *widget, gpointer data);
void on_file_menuitem_activate (GtkWidget *widget, gpointer data);
gint count_all_downloads(void);


#endif
