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
 
#ifndef EPHY_GWGET_EXTENSION_H
#define EPHY_GWGET_EXTENSION_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define EPHY_TYPE_GWGET_EXTENSION (ephy_gwget_extension_get_type ())
#define EPHY_GWGET_EXTENSION(o) (G_TYPE_CHECK_INSTANCE_CAST ((o, EPHY_TYPE_GWGET_EXTENSION, EphyGwgetExtension))
#define EPHY_GWGET_EXTENSION_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_GWGET_EXTENSION, EphyGwgetExtensionClass))
#define EPHY_IS_GWGET_EXTENSION(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_GWGET_EXTENSION))
#define EPHY_IS_GWGET_EXTENSION_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_GWGET_EXTENSION))
#define EPHY_GWGET_EXTENSION_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_GWGET_EXTENSION, EphyGwgetExtensionClass))

typedef struct EphyGwgetExtension EphyGwgetExtension;
typedef struct EphyGwgetExtensionClass EphyGwgetExtensionClass;
typedef struct EphyGwgetExtensionPrivate EphyGwgetExtensionPrivate;
	
struct EphyGwgetExtensionClass
{
  GObjectClass parent_class;
};

struct EphyGwgetExtension
{
  GObject parent_instance;

  EphyGwgetExtensionPrivate *priv;
};

GType ephy_gwget_extension_get_type (void);
GType ephy_gwget_extension_register_type (GTypeModule *module);

G_END_DECLS
	
#endif
