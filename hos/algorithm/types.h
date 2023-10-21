#ifndef HOS_TYPES_H
#define HOS_TYPES_H

#include <stdbool.h>
#include <stddef.h>

typedef char  *error;
typedef int    ready;
#define SUCCESS (void*)0

typedef struct hos_Channel        hos_Channel;
typedef struct hos_CircularBuffer hos_CircularBuffer;
typedef struct hos_StickyByte     hos_StickyByte;

#endif
