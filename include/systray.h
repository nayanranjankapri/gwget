#ifndef _SYSTRAY_H
#define _SYSTRAY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <gtk/gtkplug.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif
#ifdef HAVE_NOTIFY
#include <libnotify/notify.h>
#endif

void systray_load(void);
void set_icon_newdownload(void);
void set_icon_downloading(void);
void set_icon_idle(void);
void gwget_tray_notify(gchar *primary, gchar *secondary, gchar *icon_nam);


#ifdef __cplusplus
}
#endif

#endif /* _SYSTRAY_H */
