
#ifndef __GWGET_APPLICATION_SERVER_H
#define __GWGET_APPLICATION_SERVER_H

#include <GNOME_Gwget.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-object.h>

G_BEGIN_DECLS

#define GWGET_APPLICATION_SERVER_TYPE         (gwget_application_server_get_type ())
#define GWGET_APPLICATION_SERVER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GWGET_APPLICATION_SERVER_TYPE, GwgetApplicationServer))
#define GWGET_APPLICATION_SERVER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), GWGET_APPLICATION_SERVER_TYPE, GwgetApplicationServerClass))
#define GWGET_APPLICATION_SERVER_IS_OBJECT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GWGET_APPLICATION_SERVER_TYPE))
#define GWGET_APPLICATION_SERVER_IS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GWGET_APPLICATION_SERVER_TYPE))
#define GWGET_APPLICATION_SERVER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GWGET_APPLICATION_SERVER_TYPE, GwgetApplicationServerClass))

typedef struct
{
        BonoboObject parent;
} GwgetApplicationServer;

typedef struct
{
        BonoboObjectClass parent_class;

        POA_GNOME_Gwget_Application__epv epv;
} GwgetApplicationServerClass;

GType          gwget_application_server_get_type (void);

BonoboObject  *gwget_application_server_new      (GdkScreen *screen);

G_END_DECLS

#endif /* __GWGET_APPLICATION_SERVER_H */
