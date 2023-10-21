#include "kernel.h"
#include "public.h"

const hos_Module *modules[40] = {0};
int               modulesz    = 0;

void hos_Module_register(const hos_Module *module) {
    modules[modulesz++] = module;
    if (module->Init) {
        module->Init();
    }
}
void hos_kernel_run_preInterruptions(void) {
    for (int i=0; i<modulesz; i++) {
        if (modules[i]->PreInterruptions) {
            modules[i]->PreInterruptions();
        }
    }
}
void hos_kernel_run_interruptions (uint8_t interruption) {
    for (int i=0; i<modulesz; i++) {
        if (modules[i]->Interruption) {
            modules[i]->Interruption(interruption);
        }
    }
}
void hos_event (const char event[], ...) {
    va_list va;
    for (int i=0; i<modulesz; i++) {
        if (modules[i]->Event) {
            va_start(va, event);
            modules[i]->Event(event, va);
            va_end(va);
        }
    }
}
void hos_shell (void) {
    for (int i=0; i<modulesz; i++) {
        if (modules[i]->Shell) {
            return modules[i]->Shell();
        }
    }
    hos_event("NoShell");
}
bool hos_command (int argc, char *argv[], int *res) {
    bool found = false;
    for (int i=0; i<modulesz; i++) {
        if ((modules[i]->Command && modules[i]->Command(argc, argv, res))) {
            found = true;
        }
    }
    return found;
}
void hos_log(const char s[]) {
    for (int i=0; i<modulesz; i++) {
        if (modules[i]->Log) {
            modules[i]->Log(s);
        }
    }
}
