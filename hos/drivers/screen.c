#define MODULE_IMPLEMENTED_FOR_32BITS
#include "../kernel/public.h"
#include "screen.h"

#define HOS_COLS 80
#define HOS_ROWS 25

int  HOS_START_Y = 0;
int  HOS_START_X = 0;
int  HOS_XPOS    = 0;
int  HOS_YPOS    = 0;

static int  HOS_XPOS_S  = 0;
static int  HOS_YPOS_S  = 0;
static char buf[HOS_COLS][HOS_ROWS] = {0};
static hos_Port cursorPortCommand = {0x3D4, 8, false};
static hos_Port cursorPortData    = {0x3D5, 8, false};


void hos_screen_set(int x, int y, char c, bool ptr) {
    uint16_t loc = y * HOS_COLS + x;
    if (c) {
        unsigned short *m = ((unsigned short*) 0xb8000) + loc;
        *m = (( (*m) & 0xff00 ) | c);
        buf[x][y] = c;
    }
    if (ptr) {
        uint8_t  loch = (loc >> 8) & 0xFF;
        uint8_t  locl = loc & 0xFF;
        hos_Port_write(&cursorPortCommand, 14);
        hos_Port_write(&cursorPortData, loch);
        hos_Port_write(&cursorPortCommand, 15);
        hos_Port_write(&cursorPortData, locl);
    }
}
char hos_screen_get(int x, int y) {
    return (buf[x][y])?buf[x][y]:' ';
}
void hos_screen_clr(int s, int y) {
    for (int x=s; x<HOS_COLS; x++) {
        hos_screen_set(x, y, ' ', false);
    }
}
void hos_screen_push(int x, int y) {
    HOS_XPOS_S = HOS_XPOS;
    HOS_YPOS_S = HOS_YPOS;
    HOS_XPOS   = x;
    HOS_YPOS   = y;
}
void hos_screen_pop(int *x) {
    if (x) *x = HOS_XPOS;
    HOS_XPOS = HOS_XPOS_S; 
    HOS_YPOS = HOS_YPOS_S;
}
void hos_screen_color (int x0, int y0, int x1, int y1, uint8_t clr) {
    for (int y=y0; y<y1; y++) {
        for (int x=x0; x<x1; x++) {
            uint16_t loc = y * HOS_COLS + x;
            unsigned short *m = ((unsigned short*) 0xb8000) + loc;
            *m = (*m & 0x00ff) | (clr << 8);
        }
    }
}


void hos_clear(void) {
    for (int y=HOS_START_Y; y<HOS_ROWS; y++) {
        hos_screen_clr(HOS_START_X, y);
    }
    HOS_XPOS = HOS_START_X;
    HOS_YPOS = HOS_START_Y;
}
void hos_putChar(char _c, bool _ptr) {
    switch (_c) {
    case '\n':
        hos_screen_clr(HOS_XPOS, HOS_YPOS);
        HOS_XPOS = HOS_START_X;
        HOS_YPOS++;
        break;
    case '\r':
        HOS_XPOS = HOS_START_X;
        break;
    case 0x7f:
        if (HOS_XPOS > 0) {
            hos_screen_set(--HOS_XPOS, HOS_YPOS, ' ', _ptr);
        }
        break;
    default:
        hos_screen_set(HOS_XPOS, HOS_YPOS, _c, false);
        HOS_XPOS++;
        if (HOS_XPOS == HOS_COLS) {
            HOS_XPOS=HOS_START_X;
            HOS_YPOS++;
        }
        break;
    }
    if (HOS_YPOS >= HOS_ROWS) {
        for (int i=HOS_START_Y; i<HOS_ROWS-1; i++) {
            for (int j=HOS_START_X; j<HOS_COLS; j++) {
                hos_screen_set(j, i, hos_screen_get(j, i+1), false);
            }
        }
        HOS_YPOS = HOS_ROWS-1;
        HOS_XPOS = HOS_START_X;
        hos_screen_clr(0, HOS_YPOS);
    }
    hos_screen_set(HOS_XPOS, HOS_YPOS, 0, _ptr);
}
void hos_print(const char _s[], bool _ptr) {
    for (int i=0; _s[i]; i++) {
        hos_putChar(_s[i], _ptr);
    }
}
void hos_put8(uint8_t key, bool _ptr) {
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    hos_print(foo, _ptr);
}
void hos_put16(uint16_t key, bool _ptr) {
    hos_put8((key >> 8) & 0xFF, _ptr);
    hos_put8( key & 0xFF, _ptr);
}
void hos_put32(uint32_t key, bool _ptr) {
    hos_put8((key >> 24) & 0xFF, _ptr);
    hos_put8((key >> 16) & 0xFF, _ptr);
    hos_put8((key >> 8) & 0xFF, _ptr);
    hos_put8( key & 0xFF, _ptr);
}



static void _log(const char _s[]) {
    hos_print(_s, true);
}
hos_Module hos_Screen_Module = {
    .Log = _log
};
