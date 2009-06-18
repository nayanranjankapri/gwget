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
 

#include <config.h>

#include <gnome.h>
#include <string.h>
#include <gconf/gconf-client.h>
#include <locale.h>
#include <libgnomeui/libgnomeui.h>
#include <gio/gio.h>

#include "main_window_cb.h"
#include "main_window.h"
#include "gwget-application.h"

#ifdef ENABLE_DBUS
#include <dbus/dbus-glib-bindings.h>
#endif

static gchar *destination_dir;
static const char **url_arguments = NULL;

static const GOptionEntry goption_options [] =
{
    { "force-tray-only", 't', 0, G_OPTION_ARG_NONE, &(gwget_pref.trayonly), N_("Launch gwget in the notification area"), NULL },
    { "destination-dir", 'd', 0, G_OPTION_ARG_STRING, &destination_dir, N_("Destination directory for the download"), NULL },
    { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &url_arguments, NULL, N_("[URL]") },
    { NULL}
};

typedef struct {
	int argc;
	char **argv;
} Args;

#ifdef ENABLE_DBUS

#ifndef HAVE_GTK_WINDOW_PRESENT_WITH_TIME
static guint32
get_startup_time (void)
{
	const char *envvar, *timestamp;
	unsigned long value;
	char *end;

	envvar = getenv ("DESKTOP_STARTUP_ID");

	if (envvar == NULL)
		return 0;

/* DESKTOP_STARTUP_ID is of form "<unique>_TIME<timestamp>".
 *
 * <unique> might contain a T but <timestamp> is an integer.  As such,
 * the last 'T' in the string must be the start of "TIME".
 */
	timestamp = rindex (envvar, 'T');

/* Maybe the word "TIME" was not found... */
	if (timestamp == NULL || strncmp (timestamp, "TIME", 4))
		return 0;

	timestamp += 4;

/* strtoul sets errno = ERANGE on overflow, but it is not specified
 * if it sets it to 0 on success.  Doing so ourselves is the only
 * way to know for sure.
 */
	errno = 0;
	value = strtoul (timestamp, &end, 10);

/* unsigned long might be 64bit, so double-check! */
	if (errno != 0 || *end != '\0' || value > G_MAXINT32)
		return 0;

	return value;
}
#endif

static gboolean
load_files_remote (const char **files)
{
	int i;
	GError *error = NULL;
	DBusGConnection *connection;
	gboolean result = FALSE;
	DBusGProxy *remote_object;
#ifdef HAVE_GTK_WINDOW_PRESENT_WITH_TIME
	GdkDisplay *display;
#endif
	guint32 timestamp;

#ifdef HAVE_GTK_WINDOW_PRESENT_WITH_TIME
	display = gdk_display_get_default();
	timestamp = gdk_x11_display_get_user_time (display);
#else
	/* Fake it for GTK+2.6 */
	timestamp = get_startup_time ();
#endif

connection = dbus_g_bus_get (DBUS_BUS_STARTER, &error);
	if (connection == NULL) {
		g_warning ("%s", error->message);
		g_error_free (error);	

		return FALSE;
	}

	remote_object = dbus_g_proxy_new_for_name (connection,
						   "org.gnome.gwget.ApplicationService",
                                                   "/org/gnome/gwget/Gwget",
                                                   "org.gnome.gwget.Application");
	if (!files) {
        	if (!dbus_g_proxy_call (remote_object, "OpenWindow", &error,
					G_TYPE_UINT, timestamp,
					G_TYPE_INVALID,
					G_TYPE_INVALID)) {
			g_warning ("%s", error->message);
			g_clear_error (&error);
			return FALSE;
		}
		return TRUE;
	}

	for (i = 0; files[i]; i++) {
		char *uri;
		char *dest_dir;
		GFile *file;
		
		file = g_file_new_for_commandline_arg (files[i]);
		uri = g_file_get_uri (file);

		if (destination_dir) {
        		dest_dir = destination_dir;
        	} 
		if (!dbus_g_proxy_call (remote_object, "OpenURIDest", &error,
					G_TYPE_STRING, uri,
					G_TYPE_STRING, dest_dir,
					G_TYPE_UINT, timestamp,
					G_TYPE_INVALID,
					G_TYPE_INVALID)) {
			g_warning ("%s", error->message);
			g_clear_error (&error);
			g_free (uri);
			continue;
		}
		g_free (uri);
		g_object_unref (file);
		result = TRUE;
        }

	gdk_notify_startup_complete ();

	return result;
}
#endif /* ENABLE_DBUS */
	
static void
load_urls (const char **urls)
{
        int i;
        GwgetData *gwgetdata;
        gchar *url;

        main_window();
    
        if (!urls) {
                return;
        }

        for ( i = 0; urls[i]; i++) {
		GFile *file;
        	
		file = g_file_new_for_commandline_arg (urls[i]);
        	
                url = g_file_get_uri (file);
                gwgetdata = gwget_data_new ((gchar *)url);
                if (destination_dir) {
                        gwgetdata->dir = destination_dir;
                }
                gwget_data_add_download(gwgetdata);
                gwget_data_start_download(gwgetdata);
                g_object_unref (file);
        }
}                
	

int main(int argc,char *argv[])
{
	GnomeProgram *program;
	GOptionContext *context;
	
	context = g_option_context_new (_("Gwget Download Manager"));
	
#ifdef ENABLE_NLS
	/* Initialize the i18n stuff */	
	bindtextdomain (GETTEXT_PACKAGE, GNOME_GWGET_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	g_option_context_add_main_entries (context, goption_options, GETTEXT_PACKAGE);
	g_option_context_set_translation_domain (context, GETTEXT_PACKAGE);
#else
        g_option_context_add_main_entries (context, goption_options, NULL);
#endif
	setlocale(LC_ALL, "");
	

	gwget_init_pref(&gwget_pref);
	program = gnome_program_init(PACKAGE, VERSION, 
				LIBGNOMEUI_MODULE, argc, argv,
			        GNOME_PARAM_GOPTION_CONTEXT, context, 
				GNOME_PARAM_HUMAN_READABLE_NAME, _("Gwget"),
				GNOME_PARAM_APP_DATADIR, GNOME_GWGET_LOCALEDIR,
                              	NULL);
	
#ifdef ENABLE_DBUS
	if (!gwget_application_register_service (GWGET_APP)) {
		if (load_files_remote (url_arguments)) {
			return 0;
		}
	} else {
		/* enable_metadata = TRUE; */
	}
#endif
	
	g_set_application_name (_("Gwget Download Manager"));

        load_urls (url_arguments);	
	
	gtk_main();
	
	gnome_accelerators_sync ();
	
	g_object_unref (program);
	
	return (0);
}
