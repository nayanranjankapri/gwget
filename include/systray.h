#ifndef _SYSTRAY_H
#define _SYSTRAY_H

#ifdef __cplusplus
extern "C"
{
#endif

void systray_load(void *windowToHide);
void set_icon_downloading(void);
void set_icon_idle(void);

#ifdef __cplusplus
}
#endif

#endif /* _SYSTRAY_H */
