#include "sync.h"
#include <stdarg.h>

/* ----------------------------------------------------------------
 * ---- GENERIC ---------------------------------------------------
 * ---------------------------------------------------------------- */

ready hos_wait(int *onum, int inum, ...) {
    va_list va;
    
    bool found = false;

    va_start(va, inum);
    for(int i=0; i<inum && !found; i++) {
        if (va_arg(va, ready)) {
            if (onum) *onum = i;
            found = true;
            break;
        }
    }
    va_end(va);

    if (found) {
        return true;
    } else {
        asm volatile("hlt");
        return false;
    }
}

/* ----------------------------------------------------------------
 * ---- CHANNELS --------------------------------------------------
 * ---------------------------------------------------------------- */
void hos_Channel_attach(hos_Channel *c, void *udata, error (*copy) (void *v, char c)) {
    c->chans[c->chansz].udata = udata;
    c->chans[c->chansz].copy  = copy;
    c->chansz++;
}
error hos_Channel_putc (hos_Channel *c, char  b) {
    for (size_t i=0; i<c->chansz; i++) {
        c->chans[i].copy(c->chans[i].udata, b);    
    }
    return NULL;
}

/* ----------------------------------------------------------------
 * ---- CIRCULAR BUFFER -------------------------------------------
 * ---------------------------------------------------------------- */

error hos_CircularBuffer_putc(hos_CircularBuffer *cb, char c) {
    if (cb->count == cb->size) {
        return "overflow";
    }
    cb->buf[cb->end] = c;
    cb->end = (cb->end + 1) % cb->size;
    cb->count++;
    return 0;
}

ready hos_CircularBuffer_getc(hos_CircularBuffer *cb, char *c) {
    if (cb->count == 0) {
        return false;
    }
    *c = cb->buf[cb->start];
    cb->start = (cb->start + 1) % cb->size;
    cb->count--;
    return true;
}

ready hos_CircularBuffer_wait(hos_CircularBuffer *cb) {
    return cb->count != 0;
}

void  hos_Channel_attach_CircularBuffer (hos_Channel *c, hos_CircularBuffer *cb) {
    hos_Channel_attach(c, cb, (void*)hos_CircularBuffer_putc);
}

/* ----------------------------------------------------------------
 * ---- STICKY BYTE -----------------------------------------------
 * ---------------------------------------------------------------- */

error hos_StickyByte_putc (hos_StickyByte *sb, char c) {
    if (!sb->IsIn && sb->InByte == c) {
        sb->IsIn = true;
    } else if (sb->IsIn && sb->OutByte == c) {
        sb->IsIn = false;
    }
    return NULL;
}

bool hos_StickyByte_isActive(hos_StickyByte *sb) {
    return sb->IsIn;
}

void hos_Channel_attach_StickyByte (hos_Channel *c, hos_StickyByte *sb) {
    hos_Channel_attach(c, sb, (void*)hos_StickyByte_putc);
}
