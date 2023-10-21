#ifndef HOS_KERNEL_PUBLIC_H
#define HOS_KERNEL_PUBLIC_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#ifdef MODULE_IMPLEMENTED_FOR_32BITS
#  include "../kernel/32.h"
#endif

typedef struct hos_Module {
    void (*Event)            (const char event[], va_list va);
    void (*Init)             (void);
    void (*PreInterruptions) (void);
    void (*Interruption)     (uint8_t interruption);
    bool (*Command)          (int argc, char *argv[], int *res);
    void (*Shell)            (void);
    void (*Log)              (const char _msg[]);
} hos_Module;

extern bool hos_command (int argc, char *argv[], int *res);
extern void hos_log     (const char s[]);
extern void hos_event   (const char event[], ...);
extern void hos_shell   (void);

#endif
