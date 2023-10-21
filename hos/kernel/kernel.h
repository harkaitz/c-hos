#ifndef HOS_KERNEL_PRIVATE_H
#define HOS_KERNEL_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct hos_Module hos_Module;

void hos_Module_register             (const hos_Module *module);
void hos_kernel_run_preInterruptions (void);
void hos_kernel_run_interruptions    (uint8_t interruption);



#endif
