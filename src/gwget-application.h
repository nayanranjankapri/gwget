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
 
 
#ifndef GWGET_APPLICATION_H
#define GWGET_APPLICATION_H

#include <glib/gerror.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _GwgetApplication GwgetApplication;
typedef struct _GwgetApplicationClass GwgetApplicationClass;
typedef struct _GwgetApplicationPrivate GwgetApplicationPrivate;

#define GWGET_TYPE_APPLICATION			(gwget_application_get_type ())
#define GWGET_APPLICATION(object)			(G_TYPE_CHECK_INSTANCE_CAST((object), GWGET_TYPE_APPLICATION, GwgetApplication))
#define GWGET_APPLICATION_CLASS(klass)		(G_TYPE_CHACK_CLASS_CAST((klass), GWGET_TYPE_APPLICATION, GwgetApplicationClass))
#define GWGET_IS_APPLICATION(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), GWGET_TYPE_APPLICATION))
#define GWGET_IS_APPLICATION_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), GWGET_TYPE_APPLICATION))
#define GWGET_APPLICATION_GET_CLASS(object)	(G_TYPE_INSTANCE_GET_CLASS((object), GWGET_TYPE_APPLICATION, GwgetApplicationClass))

#define GWGET_APP					(gwget_application_get_instance ())

struct _GwgetApplication {
	GObject base_instance;
	
	
	gchar *last_chooser_uri;
};

struct _GwgetApplicationClass {
	GObjectClass base_class;
};

GType	          gwget_application_get_type	     (void);
GwgetApplication    *gwget_application_get_instance        (void);
gboolean          gwget_application_register_service    (GwgetApplication   *application);
void	          gwget_application_shutdown	     (GwgetApplication   *application);


gboolean          gwget_application_open_window         (GwgetApplication   *application,
						      guint32         timestamp,
						      GError         **error);
gboolean          gwget_application_open_uri            (GwgetApplication   *application,
		                      const char      *uri,
						      guint32         timestamp,
						      GError         **error);
gboolean          gwget_application_open_uri_with_dest (GwgetApplication   *application,
		                      const char      *uri,
		                      const char      *destination_dir,
						              guint32         timestamp,
						              GError         **error);


void	          gwget_application_open_uri_list       (GwgetApplication   *application,
		  			              GSList          *uri_list,
    						      guint32          timestamp);

void 		  gwget_application_set_chooser_uri     (GwgetApplication *application, 
						      gchar *uri);
const gchar	 *gwget_application_get_chooser_uri     (GwgetApplication *application);

G_END_DECLS

#endif /* !GWGET_APPLICATION_H */
