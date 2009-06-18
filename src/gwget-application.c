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
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gwget-application.h"


#include <libgnomeui/gnome-client.h>

#ifdef ENABLE_DBUS
#include "gwget-application-service.h"
#include <dbus/dbus-glib-bindings.h>
#endif

#include <gtk/gtk.h>
#include <string.h>
#include "gwget_data.h"

G_DEFINE_TYPE (GwgetApplication, gwget_application, G_TYPE_OBJECT);

#define GWGET_APPLICATION_GET_PRIVATE(object) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((object), GWGET_TYPE_APPLICATION, GwgetApplicationPrivate))

#define APPLICATION_SERVICE_NAME "org.gnome.gwget.ApplicationService"


#ifdef ENABLE_DBUS
gboolean
gwget_application_register_service (GwgetApplication *application)
{
	DBusGConnection *connection;
	DBusGProxy *driver_proxy;
	GError *err = NULL;
	guint request_name_result;

	connection = dbus_g_bus_get (DBUS_BUS_STARTER, &err);
	if (connection == NULL) {
		g_warning ("Service registration failed.");
		g_error_free (err);

		return FALSE;
	}

	driver_proxy = dbus_g_proxy_new_for_name (connection,
						  DBUS_SERVICE_DBUS,
						  DBUS_PATH_DBUS,
						  DBUS_INTERFACE_DBUS);

	if (!org_freedesktop_DBus_request_name (driver_proxy,
                                        	APPLICATION_SERVICE_NAME,
						DBUS_NAME_FLAG_DO_NOT_QUEUE,
						&request_name_result, &err)) {
		g_warning ("Service registration failed.");
		g_clear_error (&err);
	}

	if (request_name_result == DBUS_REQUEST_NAME_REPLY_EXISTS) {
		return FALSE;
	}

	dbus_g_object_type_install_info (GWGET_TYPE_APPLICATION,
					 &dbus_glib_gwget_application_object_info);

	dbus_g_connection_register_g_object (connection,
					     "/org/gnome/gwget/Gwget",
                                             G_OBJECT (application));

	return TRUE;
}
#endif /* ENABLE_DBUS */


static gint
save_session (GnomeClient *client, gint phase, GnomeSaveStyle save_style, gint shutdown,
	      GnomeInteractStyle interact_style, gint fast, GwgetApplication *application)
{
	char **restart_argv;
	int argc = 0;

	restart_argv = g_new (char *, 1);
	restart_argv[0] = g_strdup ("gwget");
	gnome_client_set_restart_command (client, argc, restart_argv);

	return TRUE;
}

static void
removed_from_session (GnomeClient *client, GwgetApplication *application)
{
	gwget_application_shutdown (application);
}

static void
init_session (GwgetApplication *application)
{
	GnomeClient *client;

	client = gnome_master_client ();

	g_signal_connect (client, "save_yourself",
			  G_CALLBACK (save_session), application);	
	g_signal_connect (client, "die",
			  G_CALLBACK (removed_from_session), application);
}

gboolean
gwget_application_open_window (GwgetApplication  *application,
                           guint32         timestamp,
                           GError        **error)
{

       return TRUE;
}

gboolean
gwget_application_open_uri (GwgetApplication  *application,
			 const char     *url,
			 guint32         timestamp,
			 GError        **error)
{
	GwgetData *gwgetdata;

	gwgetdata = gwget_data_new ((gchar *)url);

	gwget_data_add_download(gwgetdata);
	gwget_data_start_download(gwgetdata);

	return TRUE;
}

gboolean
gwget_application_open_uri_with_dest (GwgetApplication  *application,
			 const char     *url,
			 const char	*destination_dir,
			 guint32         timestamp,
			 GError        **error)
{
	GwgetData *gwgetdata;
	
	gwgetdata = gwget_data_new ((gchar *)url);
	
	if (strlen(destination_dir)!=0) {
		gwgetdata->dir = (gchar *)destination_dir;
        } else {
        	gwgetdata->dir = gwget_pref.download_dir;
        }
	
	gwget_data_add_download(gwgetdata);
	gwget_data_start_download(gwgetdata);

	return TRUE;
}


GwgetApplication *
gwget_application_get_instance (void)
{
	static GwgetApplication *instance;

	if (!instance) {
		instance = GWGET_APPLICATION (g_object_new (GWGET_TYPE_APPLICATION, NULL));
	}

	return instance;
}

void
gwget_application_shutdown (GwgetApplication *application)
{
	
	g_free (application->last_chooser_uri);
	g_object_unref (application);
	
	gtk_main_quit ();
}


static void
gwget_application_class_init (GwgetApplicationClass *gwget_application_class)
{
}

static void
gwget_application_init (GwgetApplication *gwget_application)
{
	init_session (gwget_application);

}


void gwget_application_set_chooser_uri (GwgetApplication *application, gchar *uri)
{
	g_free (application->last_chooser_uri);
	application->last_chooser_uri = g_strdup (uri);
}

const gchar* gwget_application_get_chooser_uri (GwgetApplication *application)
{
	return application->last_chooser_uri;
}
