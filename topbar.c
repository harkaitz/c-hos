#include "hos/hos.h"

static int xpos = 0;

static void _init(void) {
    hos_screen_clr(0,0);
    HOS_START_Y = 1;
    hos_clear();

    hos_screen_push(0,0);
    hos_print("HOS - Harka's Operating System ", false);
    hos_screen_pop(&xpos);
    
    hos_screen_color(0,0, xpos,1, 0x4f);
    hos_screen_color((xpos+=5),0,80,1, 0x90);

    hos_screen_push(xpos, 0);
    hos_print(" Status:", false);
    hos_screen_pop(&xpos);
    
    hos_screen_set(xpos+1, 0, '-', false); /* W or I (interrupt?) */
    hos_screen_set(xpos+2, 0, '-', false); /* Interruption type.  */
}

static void _event(const char event[], va_list va) {
    if (!strcmp(event, "InsideMainLoop")) {
        hos_screen_set(xpos+1, 0, 'W', false);
        hos_screen_set(xpos+2, 0, '-', false);
    } else if (!strcmp(event, "Shift")) {
        hos_screen_set(xpos+3, 0, va_arg(va, int), false);
    } else if (!strcmp(event, "Mayus")) {
        hos_screen_set(xpos+4, 0, va_arg(va, int), false);
    } else if (!strcmp(event, "Keyboard")) {
        char key = va_arg(va, int);
        hos_screen_push(xpos+5, 0);
        hos_put8(key, false);
        hos_screen_pop(NULL);
    } 
}

static void _interruption(uint8_t interruption) {
    hos_screen_set(xpos+1, 0, 'I', false);
    switch (interruption) {
    case 0x20: hos_screen_set(xpos+2, 0, 'C', false); break;
    case 0x21: hos_screen_set(xpos+2, 0, 'K', false); break;
    default:   hos_screen_set(xpos+2, 0, 'U', false); break;
    }
}

hos_Module hos_TopBar_Module = {
    .Event        = _event,
    .Init         = _init,
    .Interruption = _interruption
};
