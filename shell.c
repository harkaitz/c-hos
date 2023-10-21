#include "hos/hos.h"

static void _shell(void) {
    char  line[512] = {0};
    int   argc;
    char *argv[21];
    bool  found;
    int   ret;

    hos_log("Initialising Shell ...\n");
    hos_print("\n", true);
    while(1) {
        /* Read line. */
        hos_print(": ", true);
        hos_getLine(line, sizeof(line)-1);
        hos_event("InsideMainLoop");
        
        /* Split line. */
        bool inword = false; argc = 0;
        for(char *c = line; *c; c++) {
            if (!inword && *c != ' ') {
                argv[argc++] = c;
                inword = true;
                continue;
            }
            if (inword && *c == ' ') {
                *c = '\0';
                inword = false;
                continue;
            }
        }

        /* Empty command. */
        if (argc==0) {
            continue;
        }
        
        /* Execute command. */
        found = hos_command(argc, argv, &ret);
        if (!found) {
            hos_print("ERROR: Command not found.\n", true);
        }
        
    }
}

static bool _command(int argc, char *argv[], int *ret) {
    if (!strcmp(argv[0], "help")) {
        hos_print("echo MSG : Print a message to the terminal.\n", true);
        return true;
    } else if (!strcmp(argv[0], "echo")) {
        for (int i=1; i<argc; i++) {
            if (i!=1) hos_putChar(' ', true);
            hos_print(argv[i], true);
        }
        hos_putChar('\n', true);
        *ret = 0;
        return true;
    } else {
        return false;
    }
}

hos_Module hos_Shell_Module = {
    .Shell = _shell,
    .Command = _command
};
