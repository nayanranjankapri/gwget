
#include <config.h>
#include <gnome.h>
#include "main_window.h"
#include "eggtrayicon.h"
#include "gwget_data.h"
#include "systray.h"

static GtkWidget *systray_set_image(void);
static GdkPixbuf *systray_pixbuf_new_from_file(const gchar *filename);
static void systray_embedded(GtkWidget *widget, gpointer data);
static void systray_destroyed(GtkWidget *widget, gpointer data);
static void systray_clicked(GtkWidget *widget, GdkEventButton *event, void *data);
static gboolean systray_generate_menu(GdkEventButton *event);

void 
systray_load(void) 
{
	GtkWidget *eventbox,*image;
		
	/* tray icon */
	tray_icon = egg_tray_icon_new("gwget");
	eventbox = gtk_event_box_new();
	tray_tooltip = gtk_tooltips_new();
	
	
	image=systray_set_image();
	
	gtk_container_add(GTK_CONTAINER(eventbox),image);
	gtk_container_add(GTK_CONTAINER(tray_icon),eventbox);
	
	gtk_widget_show_all(GTK_WIDGET(tray_icon));	
	g_signal_connect(G_OBJECT(tray_icon), "embedded", G_CALLBACK(systray_embedded), NULL);
	g_signal_connect(G_OBJECT(tray_icon), "destroy", G_CALLBACK(systray_destroyed), NULL);
	g_signal_connect(G_OBJECT(eventbox), "button-press-event", G_CALLBACK(systray_clicked), NULL);
	egg_tray_icon_send_message(tray_icon,1000,"gwget",7);
}

static GtkWidget *
systray_set_image(void) 
{
	gint w = 0;
  	gint h = 0;

  	GdkPixbuf *pb = NULL;
  	GdkPixbuf *pb_scaled = NULL;
	gchar *file;
	GtkWidget *image;
	
	file = g_strdup_printf("%s/%s",DATADIR,"gwget.xpm");
	
	if((pb = systray_pixbuf_new_from_file(file)) == NULL) {
		g_warning("systray_set_image: pixbuf was NULL\n");
		return NULL;
	}
    
	image = gtk_image_new();
  	/* get size */
	gtk_icon_size_lookup(GTK_ICON_SIZE_LARGE_TOOLBAR, &w, &h);

	/* scale the image */
	pb_scaled = gdk_pixbuf_scale_simple(pb, w, h, GDK_INTERP_BILINEAR);

	/* set the image */
	gtk_image_set_from_pixbuf(GTK_IMAGE(image), pb_scaled);

	/* clean up */
 	g_object_unref(G_OBJECT(pb));

	return image;
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
systray_generate_menu(GdkEventButton *event)
{
	GtkWidget *menu = NULL;
	GtkWidget *item = NULL;

	if(event == NULL) {
		g_warning("systray_generate_menu: gdk event was NULL");
		return FALSE;
	}
	
	menu = gtk_menu_new();
	
	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	
	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate", 
			    GTK_SIGNAL_FUNC(gwget_quit), 
			    NULL);
	/* show */
	gtk_widget_show_all(GTK_WIDGET(menu));
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);

	/* clean up */
	gtk_object_sink(GTK_OBJECT(menu));

	return TRUE;

}


static void 
systray_embedded(GtkWidget *widget, gpointer data)
{
	gwget_pref.docked = TRUE;
	g_message("docket embedded");
}

static void 
systray_destroyed(GtkWidget *widget, gpointer data)
{
	g_message("docket destroyed");
}

static void 
systray_clicked(GtkWidget *widget, GdkEventButton *event, void *data)
{
	GtkWidget *window;
	g_message("docket clicked");
	
	
	if (event->button == 3) {
		systray_generate_menu(event);
	} else {
		window = glade_xml_get_widget(xml,"main_window");
		gtk_widget_show(GTK_WIDGET(window));
	}
}
