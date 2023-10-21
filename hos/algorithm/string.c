#include "string.h"
#include <stdint.h>
#include <stdbool.h>

int strcmp(const char *_s1, const char *_s2) {
    const unsigned char *s1 = (const unsigned char *) _s1;
    const unsigned char *s2 = (const unsigned char *) _s2;
    unsigned char c1, c2;
    do {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 == '\0') return c1 - c2;
    } while (c1 == c2);
    return c1 - c2;
}


