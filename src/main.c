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
#include <gconf/gconf-client.h>
#include <locale.h>
#include <libbonobo.h>
#include <libgnomeui/libgnomeui.h>
#include "main_window_cb.h"
#include "main_window.h"
#include "gwget-application-server.h"

typedef struct {
	int argc;
	char **argv;
} Args;

BonoboObject *gwget_app_server = NULL;


static GSList *
gwget_get_command_line_data (GnomeProgram *program)
{
	GValue value = { 0, };
	poptContext ctx;
	char **args;
	GSList *data = NULL;
	int i;

	g_value_init (&value, G_TYPE_POINTER);
	g_object_get_property (G_OBJECT (program), GNOME_PARAM_POPT_CONTEXT, &value);
	ctx = g_value_get_pointer (&value);
	g_value_unset (&value);

	args = (char**) poptGetArgs(ctx);

	if (args) 
	{	
		for (i = 0; args[i]; i++) 
		{
			data = g_slist_append(data,args[i]);
		}
	}
	
	return data;
	
}

static void
gwget_handle_automation_cmdline (GnomeProgram *program)
{
	CORBA_Environment env;
	GNOME_Gwget_Application server;
	GNOME_Gwget_URIList *urls_list;
	/* List of urls to download pased in command line */
	GSList *data, *list;
	gint i;
	

		
	CORBA_exception_init (&env);
	
	server = bonobo_activation_activate_from_id ("OAFIID:GNOME_Gwget_Application",
												 0, NULL, &env);
	
	g_return_if_fail (server != NULL);
	
	data = gwget_get_command_line_data(program);
	
	if (data) 
	{
		urls_list = GNOME_Gwget_URIList__alloc ();
		urls_list->_maximum = g_slist_length (data);
		urls_list->_length = urls_list->_maximum;
		urls_list->_buffer = CORBA_sequence_GNOME_Gwget_URI_allocbuf (urls_list->_length);
		
		list = data;
		i = 0;
		while (list != NULL) 
		{
			urls_list->_buffer[i] = CORBA_string_dup ((gchar*)list->data);
			list = list->next;
			i++;
		}

		
		CORBA_sequence_set_release (urls_list, CORBA_TRUE);
		GNOME_Gwget_Application_openURLSList (server, urls_list, &env);
		
		g_slist_free (data); 
	}	
}


static gint
save_yourself_handler (GnomeClient *client, gint phase, GnomeSaveStyle save_style,
                       gint is_shutdown, GnomeInteractStyle interact_style,
                       gint is_fast, gpointer client_data)
{
	Args *args = (Args*)client_data;
	int argc = (*args).argc;
	char **argv = (*args).argv;
	
	gnome_client_set_clone_command(client, argc, argv);
	gnome_client_set_restart_command(client, argc, argv);

	return TRUE;
}

static void 
gnome_session_join(int argc,char *argv[]) {
	Args *args = g_malloc(sizeof(Args));

	(*args).argc = argc;
	(*args).argv = argv;
	
	GnomeClient* client = gnome_master_client();
		
	gnome_client_set_restart_style(client,GNOME_RESTART_IF_RUNNING);	
	gtk_signal_connect(GTK_OBJECT(client),"save_yourself",
                           GTK_SIGNAL_FUNC(save_yourself_handler),args);
        gtk_signal_connect(GTK_OBJECT(client),"die",
                           GTK_SIGNAL_FUNC(gtk_main_quit),NULL);
}

int main(int argc,char *argv[])
{
	GnomeProgram *gwget;
	CORBA_Object factory;
	
	bindtextdomain (GETTEXT_PACKAGE, GNOME_GWGET_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	setlocale(LC_ALL, "");
	
	gwget = gnome_program_init("Gwget",VERSION,LIBGNOMEUI_MODULE,argc,argv,NULL);
	gnome_session_join(argc,argv);		

	/* check whether we are running already */
	factory = bonobo_activation_activate_from_id
			("OAFIID:GNOME_Gwget_Factory",
			Bonobo_ACTIVATION_FLAG_EXISTING_ONLY,
			NULL, NULL);
	
	if (factory != NULL)
	{
		g_message("Already running\n");
		gwget_handle_automation_cmdline(gwget);
		exit(0);
	}
	
	main_window();
	
	gwget_app_server = gwget_application_server_new (gdk_screen_get_default());
	gwget_handle_automation_cmdline(gwget);
	
	gtk_main();
		
	return (0);
}
