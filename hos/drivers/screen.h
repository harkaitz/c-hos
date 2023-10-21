#ifndef HOS_SCREEN_H
#define HOS_SCREEN_H

#include <stdint.h>
#include <stdbool.h>

extern void hos_screen_set   (int x, int y, char c, bool ptr);
extern char hos_screen_get   (int x, int y);
extern void hos_screen_clr   (int s, int y);
extern void hos_screen_push  (int x, int y);
extern void hos_screen_pop   (int *x);
extern void hos_screen_color (int x0, int y0, int x1, int y1, uint8_t clr);

extern void hos_clear   (void);
extern void hos_putChar (char _c, bool ptr);
extern void hos_print   (const char _s[], bool ptr);
extern void hos_put8    (uint8_t key, bool ptr);
extern void hos_put16   (uint16_t key, bool ptr);
extern void hos_put32   (uint32_t key, bool ptr);

extern int HOS_START_Y, HOS_START_X, HOS_XPOS, HOS_YPOS;


#endif
