#ifndef HOS_KEYBOARD_H
#define HOS_KEYBOARD_H

#include <stddef.h>

extern int   hos_getChar (void);
extern char *hos_getLine (char *line, size_t linesz);

#endif
