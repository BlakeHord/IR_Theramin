#include "rpi.h"

void *memset(void *_p, int c, unsigned n) {
        char *p = _p, *e = p + n;

        while(p < e)
                *p++ = c;

        return _p;
}
