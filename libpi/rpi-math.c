#include "rpi.h"
#include "rpi-math.h"

// to get around arm lack of mod/div instruction make everying power of 2.
unsigned is_pow2(unsigned n) {
        return (n & -n) == n;
}
unsigned roundup_pow2(unsigned v) {
        if(is_pow2(v))
                return v;

        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;

        return v;
}
unsigned mod_pow2(unsigned x, unsigned n) {
        assert(is_pow2(n));
        return x & (n-1);
}
