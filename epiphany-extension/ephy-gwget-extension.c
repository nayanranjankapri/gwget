/*
 *  Copyright (C) 2003 - 2005 Christian Persch
 *  Copyright (C) FIXME
 *
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

#include "ephy-gwget-extension.h"

#include <epiphany/ephy-embed-single.h>
#include <epiphany/ephy-embed-shell.h>

#include <gmodule.h>

#define EPHY_GWGET_EXTENSION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_GWGET_EXTENSION, EphyGwgetExtensionPrivate))

struct EphyGwgetExtensionPrivate
{
};

static GObjectClass *parent_class = NULL;
static GType type = 0;

static gboolean
handle_content_cb (EphyEmbedSingle *single,
		   const char *mime_type,
		   const char *uri,
		   GObject *extension)
{
	char *command;

	command = g_strdup_printf ("gwget %s", uri);

	g_spawn_command_line_async (command, NULL);

	g_free (command);

	/* we handled the url */
	return TRUE;
}

static void
ephy_gwget_extension_init (EphyGwgetExtension *extension)
{
	GObject *single;

	/* extension->priv = EPHY_GWGET_EXTENSION_GET_PRIVATE (extension); */
	
	single = ephy_embed_shell_get_embed_single (embed_shell);
	g_signal_connect (single, "handle-content",
			  G_CALLBACK (handle_content_cb), extension);
}

static void
ephy_gwget_extension_finalize (GObject *object)
{
	EphyGwgetExtension *extension = EPHY_GWGET_EXTENSION (object);
	GObject *single;

	single = ephy_embed_shell_get_embed_single (embed_shell);
	g_signal_handlers_disconnect_by_func
		(single, G_CALLBACK (handle_content_cb), extension);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_gwget_extension_class_init (EphyGwgetExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = (GObjectClass *) g_type_class_peek_parent (klass);

	object_class->finalize = ephy_gwget_extension_finalize;

	/* g_type_class_add_private (object_class, sizeof (EphyGwgetExtensionPrivate)); */
}

GType
ephy_gwget_extension_get_type (void)
{
	return type;
}

GType
ephy_gwget_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
  	{
		sizeof (EphyGwgetExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_gwget_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyGwgetExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_gwget_extension_init
  	};
  
	type = g_type_module_register_type (module, G_TYPE_OBJECT, "GWGetExtension", &our_info, 0);
  
	return type;
}
