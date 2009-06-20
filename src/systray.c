
#include <config.h>
#include <gnome.h>
#include "main_window.h"
#include "gwget_data.h"
#include "systray.h"
#include "main_window_cb.h"

static GdkPixbuf *systray_load_icon (const gchar *filename);
static GdkPixbuf *systray_pixbuf_new_from_file(const gchar *filename);
static void systray_clicked(GtkStatusIcon *status_icon,guint button,guint activate_time,gpointer user_data);
static gboolean systray_generate_menu(guint button,guint activate_time);
static void systray_add_download(gpointer data1, gpointer data2);
static gboolean put_icon_downloading(gpointer data);

static GdkPixbuf *icon_idle, *icon_downloading, *icon_newdownload;

void 
systray_load(void) 
{
	
	tray_icon = gtk_status_icon_new();

	/* icon list */
	icon_idle = systray_load_icon("gwget-off.png");
	icon_downloading = systray_load_icon("downloading.png");
	icon_newdownload = systray_load_icon("newdownload.png");
	
	set_icon_idle();

	g_signal_connect(tray_icon,
			 "popup-menu",
			 G_CALLBACK(systray_clicked), 
			 NULL);						

	g_signal_connect(tray_icon,
			"activate", 
			G_CALLBACK(systray_clicked),
			NULL);
}

static
GdkPixbuf *systray_load_icon (const gchar *filename)
{
	gint w = 0;
  	gint h = 0;

  	GdkPixbuf *pb = NULL;
  	GdkPixbuf *pb_scaled = NULL;
	gchar *file;
	
	file = g_strdup_printf("%s/%s", DATADIR, filename);
	
	if((pb = systray_pixbuf_new_from_file(file)) == NULL) {
		g_warning("systray_load_icon: pixbuf was NULL\n");
		return NULL;
	}

 	/* get size */
	gtk_icon_size_lookup(GTK_ICON_SIZE_LARGE_TOOLBAR, &w, &h);

	/* scale the image */
	pb_scaled = gdk_pixbuf_scale_simple(pb, w, h, GDK_INTERP_BILINEAR);

	/* clean up */
 	g_object_unref(G_OBJECT(pb));
	
return pb_scaled;
}

static GdkPixbuf *
systray_pixbuf_new_from_file(const gchar *filename)
{
	GdkPixbuf *pb = NULL;
	GError *error = NULL;

	if (filename == NULL || g_utf8_strlen(filename, -1) < 1) {
		g_warning("%s: filename was NULL", __FUNCTION__);
		return NULL;
	}
  
	pb = gdk_pixbuf_new_from_file(filename, &error);

	if(pb == NULL) {
		g_warning("%s: error loading file:'%s'", __FUNCTION__, filename);
	
		/* look at error */
		if(error != NULL) {
			g_warning("%s: error domain:'%s', code:%d, message:'%s'", __FUNCTION__,
			g_quark_to_string(error->domain), error->code, error->message);
			g_error_free(error);
		}
	
		return NULL;
	}

	/* NOTE: this has a refcount of 1 it needs to be unref'd */
	return pb;
}

static gboolean 
systray_generate_menu(guint button, guint time)
{
	GtkMenu *systray_menu;
	GtkMenu *downloads_menu;
	GtkWidget *item = NULL;
	
	systray_menu = GTK_MENU (gtk_menu_new());

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(systray_menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
			    GTK_SIGNAL_FUNC(on_button_new_clicked),
			    NULL);
		
	gtk_widget_show(item);
	downloads_menu = GTK_MENU(gtk_menu_new());
	
	if (count_all_downloads() == 0) {
		item = gtk_menu_item_new_with_label(_("Nothing"));
		gtk_widget_set_sensitive(item,FALSE);
		gtk_menu_shell_append(GTK_MENU_SHELL(downloads_menu),item);
	} else {
		g_list_foreach(downloads,systray_add_download,downloads_menu);
	}
	
	item = gtk_menu_item_new_with_label(_("Downloads"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), GTK_WIDGET(downloads_menu));
	gtk_menu_shell_append(GTK_MENU_SHELL(systray_menu),item);
	
	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(systray_menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate", 
			    GTK_SIGNAL_FUNC(on_boton_pref_clicked), 
			    NULL);
	
	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(systray_menu), item);
	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(systray_menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate", 
			    GTK_SIGNAL_FUNC(gwget_quit), 
			    NULL);
	/* show */
	gtk_widget_show_all(GTK_WIDGET(systray_menu));
	gtk_menu_popup(GTK_MENU(systray_menu), NULL, NULL, NULL, NULL, button, time);

	/* clean up */
	/* gtk_object_sink(GTK_OBJECT(menu)); */

	return TRUE;

}

static void 
pop_main_window() 
{
	GtkWidget *window;
	
	window = GTK_WIDGET (gtk_builder_get_object(builder,"main_window"));
	if((gdk_window_get_state(GTK_WIDGET(window)->window) & 
				 GDK_WINDOW_STATE_ICONIFIED) || 
			         !GTK_WIDGET_VISIBLE(window)) 
		gtk_window_present (GTK_WINDOW(window));
	else 
		gtk_widget_hide (GTK_WIDGET(window));
}

static void 
systray_clicked(GtkStatusIcon *status_icon, guint button, 
		guint activate_time,gpointer user_data)
{
	if (button == 3) {
		systray_generate_menu(button,activate_time);
	} else {
		pop_main_window();
	}
}

static void
systray_add_download(gpointer data1,gpointer data2)
{
	GwgetData *gwgetdata = data1;
	GtkWidget *menu = data2;
	GtkWidget *item;
	gchar *title;
	
	title = g_strdup_printf("%s\n(%s)",gwgetdata->url,gwgetdata->state_str);
	item = gtk_menu_item_new_with_label(title);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	
}
void 
set_icon_newdownload()
{
	gtk_status_icon_set_from_pixbuf(tray_icon, icon_newdownload);
	g_timeout_add (1500, put_icon_downloading, NULL);
}

void
set_icon_downloading()
{
	gtk_status_icon_set_from_pixbuf(tray_icon, icon_downloading);
}

static gboolean
put_icon_downloading (gpointer data)
{
	gtk_status_icon_set_from_pixbuf(tray_icon, icon_downloading);
	return FALSE;
}

void
set_icon_idle()
{
	gtk_status_icon_set_from_pixbuf(tray_icon, icon_idle);
}

void
gwget_tray_notify (gchar *primary, gchar *secondary, gchar *icon_name)
{
#ifdef HAVE_NOTIFY
	if (LIBNOTIFY_VERSION_MINOR >= 3)
       		if (!notify_is_initted ())
			if (!notify_init ("gwget"))
		               return;
       NotifyNotification *notification = notify_notification_new(primary,secondary,icon_name,NULL);
       notify_notification_show(notification,NULL);
#endif

}
