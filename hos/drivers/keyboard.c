#define MODULE_IMPLEMENTED_FOR_32BITS
#include "../kernel/public.h"
#include "../algorithm/sync.h"
#include "keyboard.h"
#include "screen.h"


hos_define_Channel        (hos_KeyboardChannel);
hos_define_StickyByte     (hos_KeyboardShift, 0x2A, 0xAA);
hos_define_StickyByte     (hos_KeyboardMayus, 0X3A, 0x3A);
hos_define_CircularBuffer (hos_AsciiBuffer, 1024);

hos_Port PICKeyboardData    = { 0x60, 8, false};
hos_Port PICKeyboardCommand = { 0x64, 8, false};

int hos_getChar(void) {
    char c;
    while(!hos_wait(0, 1, hos_CircularBuffer_getc(&hos_AsciiBuffer, &c))) {}
    return c;
}

char *hos_getLine(char *line, size_t linesz) {
    for (size_t i=0; i<linesz; i++) {
        char c = hos_getChar();
        switch(c) {
        case '\n':
            line[i] = '\0';
            hos_putChar(c, true);
            return line;
        case 0x7f:
            if (i>0) {
                line[--i]='\0';
                i--;
                hos_putChar(c, true);
                continue;
            }
            break;
        }
        hos_putChar((line[i] = c), true);
    }
    while (hos_getChar()!='\n') {}
    line[0] = '\0';
    return line;
}

/* ----------------------------------------------------------------
 * ---- MODULE ----------------------------------------------------
 * ---------------------------------------------------------------- */
static error _KeyboardChannel_to_AsciiBuffer(void *ign, char c1) {
    int i1; char c2;
    char kb1[] = {
    /*       0   1     2    3    4    5    6    7    8     9      A    B    C     D     E     F*/
    /*0*/ '\0', '\0', '1', '2', '3', '4', '5', '6' , '7' , '8' , '9', '0', '\'', '\0', 0x7f, '\0',
    /*1*/ 'q' , 'w' , 'e', 'r', 't', 'y', 'u', 'i' , 'o' , 'p' , '`', '+', '\n', '\0', 'a' ,  's',
    /*2*/ 'd' , 'f' , 'g', 'h', 'j', 'k', 'l', '\0', '\0', '\0', '\0', '}', 'z' , 'x' , 'c' ,  'v',
    /*3*/ 'b' , 'n' , 'm', ',', '.', '-', '\0', '*', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0'
    };
    char kb2[] = {
    /*       0   1     2    3    4    5    6    7    8     9      A    B    C     D     E     F*/
    /*0*/ '\0', '\0', '!', '"', '\0', '$', '%', '&' , '/', '(', ')', '=', '?', '\0', 0x7f, '\0',
    /*1*/ 'Q' , 'W' , 'E', 'R', 'T' , 'Y', 'U', 'I' , 'O', 'P', '~', '+', '\n', '\0', 'A' ,  'S',
    /*2*/ 'D' , 'F' , 'G', 'H', 'J' , 'K', 'L', '\0', '\0', '\0', '\0', '}', 'Z' , 'X' , 'C' ,  'V',
    /*3*/ 'B' , 'N' , 'M', '<', '>' , '_', '\0', '*', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0'
    };
    char kb3[] = {
    /*       0   1     2    3    4    5    6    7    8     9      A    B    C     D     E     F*/
    /*0*/ '\0', '\0', '1', '2', '3', '4', '5', '6' , '7' , '8' , '9', '0', '\'', '\0', 0x7f, '\0',
    /*1*/ 'Q' , 'W' , 'E', 'R', 'T', 'Y', 'U', 'I' , 'O', 'P', '`', '+', '\n', '\0', 'A' ,  'S',
    /*2*/ 'D' , 'F' , 'G', 'H', 'J', 'K', 'L', '\0', '\0', '\0', '\0', '}', 'Z' , 'X' , 'C' ,  'V',
    /*3*/ 'B' , 'N' , 'M', ',', '.', '-', '\0', '*', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0'
    };

    
    hos_event("Shift", hos_StickyByte_isActive(&hos_KeyboardShift)?'S':'-');
    hos_event("Mayus", hos_StickyByte_isActive(&hos_KeyboardMayus)?'M':'-');
    
    if (0 < c1 && c1 < 0x40) {
        
        i1 = c1;
        if (hos_StickyByte_isActive(&hos_KeyboardShift)) {
            c2 = kb2[i1];
        } else if (hos_StickyByte_isActive(&hos_KeyboardMayus)) {
            c2 = kb3[i1];
        } else {
            c2 = kb1[i1];
        }
        if (c2) {
            return hos_CircularBuffer_putc(&hos_AsciiBuffer, c2);
        }
    }
    return NULL;
}
static void _init(void) {
    hos_Channel_attach_StickyByte (&hos_KeyboardChannel, &hos_KeyboardShift);
    hos_Channel_attach_StickyByte (&hos_KeyboardChannel, &hos_KeyboardMayus);
    hos_Channel_attach            (&hos_KeyboardChannel, NULL, _KeyboardChannel_to_AsciiBuffer);
}
static void _preinterruptions(void) {
    hos_log("Initializing Keyboard ...\n");
    while(hos_Port_read(&PICKeyboardCommand) & 0x01) {
        hos_Port_read(&PICKeyboardData);
    }
    hos_Port_write(&PICKeyboardCommand, 0xAE); // Enable interrupts.
    hos_Port_write(&PICKeyboardCommand, 0x20); // Read from the PIC.
    int status = (hos_Port_read(&PICKeyboardData) | 1) & ~0x10;
    hos_Port_write(&PICKeyboardCommand, 0x60); // Set the status.
    hos_Port_write(&PICKeyboardData, status);
    hos_Port_write(&PICKeyboardData, 0xF4); // Activate the keyboard.
}
static void _interruption(uint8_t interruption) {
    char c1;
    if (interruption == 0x21) {
        c1 = hos_Port_read(&PICKeyboardData);
        hos_event("Keyboard", c1);
        hos_Channel_putc(&hos_KeyboardChannel, c1);
    }
}
hos_Module hos_Keyboard_Module = {
    .Init             = _init,
    .PreInterruptions = _preinterruptions,
    .Interruption     = _interruption
};


