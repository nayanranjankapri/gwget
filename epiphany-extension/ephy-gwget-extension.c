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

#include "ephy-gwget-extension.h"


#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-embed-single.h>
#include <epiphany/ephy-embed-shell.h>

#include <gmodule.h>

#define EPHY_GWGET_EXTENSION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_GWGET_EXTENSION, EphyGwgetExtensionPrivate))

struct EphyGwgetExtensionPrivate
{
};

static void ephy_gwget_extension_class_init	(EphyGwgetExtensionClass *klass);
static void ephy_gwget_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_gwget_extension_init	(EphyGwgetExtension *extension);

static GObjectClass *gwget_extension_parent_class = NULL;

static GType gwget_extension_type = 0;

GType
ephy_gwget_extension_get_type (void)
{
	return gwget_extension_type;
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
  
	static const GInterfaceInfo extension_info =
	{
    	(GInterfaceInitFunc) ephy_gwget_extension_iface_init,
    	NULL,
    	NULL
  	};

	gwget_extension_type =
    	g_type_module_register_type (module, G_TYPE_OBJECT, "EphyGwgetExtension", &our_info, 0);
  
  	g_type_module_add_interface (module, gwget_extension_type, EPHY_TYPE_EXTENSION, &extension_info);

  return gwget_extension_type;

}

static void
ephy_gwget_extension_init (EphyGwgetExtension *extension)
{
  extension->priv = EPHY_GWGET_EXTENSION_GET_PRIVATE (extension);
}


static void
ephy_gwget_extension_finalize (GObject *object)
{
	G_OBJECT_CLASS (gwget_extension_parent_class)->finalize (object);
}

static void
load_status_cb (EphyEmbedSingle *shell,
				char *mime_type,
				char *uri)
{
	gchar *command;

	command = g_strdup_printf("gwget %s",uri);

	g_spawn_command_line_async (command, NULL);
	
	g_message("URI: %s\n",uri);
	
	
}

static void
impl_attach_window (EphyExtension *ext, EphyWindow *window)
{
	EphyTab *tab;
	GtkWidget *notebook;
	EphyEmbedSingle *ephy_single;
	
	ephy_single = ephy_embed_shell_get_embed_single (embed_shell);
	
	g_signal_connect (ephy_single, "handle_content",
					  G_CALLBACK (load_status_cb), window);
}

static void
impl_detach_window (EphyExtension *extension, EphyWindow *window)
{
  
}

static void
ephy_gwget_extension_iface_init (EphyExtensionIface *iface)
{
  iface->attach_window = impl_attach_window;
  iface->detach_window = impl_detach_window;
}

static void
ephy_gwget_extension_class_init (EphyGwgetExtensionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  gwget_extension_parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = ephy_gwget_extension_finalize;
  /* object_class->get_property = ephy_gwget_extension_get_property;
  object_class->set_property = ephy_gwget_extension_set_property;
  */

  g_type_class_add_private (object_class, sizeof (EphyGwgetExtensionPrivate));
}
