#ifndef HOS_SYNC_H
#define HOS_SYNC_H

#include "types.h"


ready hos_wait(int *onum, int inum, ...);

/* ----------------------------------------------------------------
 * ---- CHANNELS --------------------------------------------------
 * ---------------------------------------------------------------- */
struct hos_Channel {
    size_t chansz;
    struct {
        error (*copy) (void *v, char c);
        void  *udata;
    } chans[20];
};

#define hos_define_Channel(y) hos_Channel y = {0}

extern void  hos_Channel_attach (hos_Channel *c, void *u, error (*copy) (void *v, char c));
extern error hos_Channel_putc   (hos_Channel *c, char  b);



/* ----------------------------------------------------------------
 * ---- CIRCULAR BUFFER -------------------------------------------
 * ---------------------------------------------------------------- */

struct hos_CircularBuffer {
    int    size;
    int    start;
    int    end;
    int    count;
    char  *buf;
};


#define hos_define_CircularBuffer(x,y) \
    char x##_buf[y];                        \
    hos_CircularBuffer x = {                \
        .size  = y,                         \
        .start = 0,                         \
        .end   = 0,                         \
        .count = 0,                         \
        .buf   = x##_buf                    \
    }

extern error hos_CircularBuffer_putc   (hos_CircularBuffer *cb, char  c);
extern ready hos_CircularBuffer_getc   (hos_CircularBuffer *cb, char *c);
extern ready hos_CircularBuffer_wait   (hos_CircularBuffer *cb);
extern void  hos_Channel_attach_CircularBuffer (hos_Channel *c, hos_CircularBuffer *cb);

/* ----------------------------------------------------------------
 * ---- STICKY BYTE -----------------------------------------------
 * ---------------------------------------------------------------- */
struct hos_StickyByte {
    char InByte;
    char OutByte;
    bool IsIn;
};

#define hos_define_StickyByte(x, i, o) hos_StickyByte x = {i,o,0}

extern error hos_StickyByte_putc     (hos_StickyByte *sb, char c);
extern bool  hos_StickyByte_isActive (hos_StickyByte *sb);
extern void  hos_Channel_attach_StickyByte (hos_Channel *c, hos_StickyByte *sb);

 




#endif
