/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gwget-application-server.c
 * This file is part of gwget
 *
 * Copyright (C) 2004 David Sedeño Fernandez. Based on 
 * gedit-application-server.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-main.h>

#include "gwget-application-server.h"
#include "main_window.h"
#include "GNOME_Gwget.h"
#include "gwget_data.h"



static void gwget_application_server_class_init (GwgetApplicationServerClass *klass);
static void gwget_application_server_init (GwgetApplicationServer *a);
static void gwget_application_server_object_finalize (GObject *object);
static GObjectClass *gwget_application_server_parent_class;

static BonoboObject *
gwget_application_server_factory (BonoboGenericFactory *this_factory,
			   const char *iid,
			   gpointer user_data)
{
        GwgetApplicationServer *a;
        
        a  = g_object_new (GWGET_APPLICATION_SERVER_TYPE, NULL);

		return BONOBO_OBJECT (a);
}

BonoboObject *
gwget_application_server_new (GdkScreen *screen)
{
	BonoboGenericFactory *factory;
	char                 *display_name;
	char                 *registration_id;

	/* We must ensure an instance of gwget per screen
	 * as gwget has no multiscreen support 
	 */
	display_name = gdk_screen_make_display_name (screen);
	registration_id = bonobo_activation_make_registration_id (
					"OAFIID:GNOME_Gwget_Factory",
					display_name);
	
	factory = bonobo_generic_factory_new ("OAFIID:GNOME_Gwget_Factory",
					      gwget_application_server_factory,
					      NULL);
	
	if (!BONOBO_IS_GENERIC_FACTORY(factory))
			g_message("Cant create corba factory");
	
	g_free (display_name);
	g_free (registration_id);

	return BONOBO_OBJECT (factory);
}

static CORBA_char*
impl_gwget_application_echo (PortableServer_Servant _servant,
                                CORBA_Environment * ev)
{
	CORBA_char *ret;
		
	ret = CORBA_string_dup ("Echo test");
	
	g_message ("In Echo Function");
	
	return ret;
}

static void
impl_gwget_application_openURLSList (PortableServer_Servant _servant,
									const GNOME_Gwget_URIList * urls,
									CORBA_Environment * ev)
{
	GSList *list = NULL;
	guint i;
	gchar *url;
	GwgetData *gwgetdata;
	
	/* convert from CORBA_sequence into GList */
    for (i = 0; i < urls->_length; i++)
	{
    	list = g_slist_prepend (list, g_strdup (urls->_buffer[i]));
	}

	list = g_slist_reverse (list);

	if (list != NULL)
	{
		while (list!=NULL) {
			url = g_strdup((const gchar *)list->data);
			gwgetdata = gwget_data_create (url, gwget_pref.download_dir);
			gwget_data_start_download(gwgetdata);
			list = g_slist_next(list);
		}
		g_slist_foreach (list, (GFunc)g_free, NULL);
		g_slist_free (list);
	}
}
	

static void
impl_gwget_application_server_quit (PortableServer_Servant _servant,
                                CORBA_Environment * ev)
{	
	g_message ("quit Called\n");
	gwget_quit();
	
}

static void
gwget_application_server_class_init (GwgetApplicationServerClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	POA_GNOME_Gwget_Application__epv *epv = &klass->epv;

	gwget_application_server_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gwget_application_server_object_finalize;

	/* connect implementation callbacks */

	epv->quit = impl_gwget_application_server_quit;
	epv->echo = impl_gwget_application_echo;
	epv->openURLSList = impl_gwget_application_openURLSList;
}

static void
gwget_application_server_init (GwgetApplicationServer *c) 
{
}

static void
gwget_application_server_object_finalize (GObject *object)
{
	GwgetApplicationServer *a = GWGET_APPLICATION_SERVER (object);

	gwget_application_server_parent_class->finalize (G_OBJECT (a));
}

BONOBO_TYPE_FUNC_FULL (
	GwgetApplicationServer,
	GNOME_Gwget_Application, 
	BONOBO_TYPE_OBJECT,           
 	gwget_application_server);
