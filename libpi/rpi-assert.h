#ifndef __RPI_ASSERT_H__
#define __RPI_ASSERT_H__
#include "printf.h"

#ifndef NDEBUG
#define debug(msg, args...) \
	(printf)("%s:%s:%d:" msg, __FILE__, __FUNCTION__, __LINE__, ##args)
#else
#define debug(msg, args...) do { } while(0)
#endif

#define panic(msg, args...) do { 					\
	(printf)("PANIC:%s:%s:%d:" msg "\n", __FILE__, __FUNCTION__, __LINE__, ##args); \
	reboot();							\
} while(0)

#define assert(bool) do { if((bool) == 0) panic(#bool); } while(0)
#define AssertNow(x) switch(1) { case (x): case 0: ; }

// #define check_byte_off(s, field, n) AssertNow(offsetof(s,field) == n)

// check bitfield positions.
#define check_bit_pos(_struct, field, n) do {                           \
        union _u {                                                      \
                _struct s;                                              \
                unsigned u;                                             \
        } x = (union _u) { .s = (_struct){ .field = 1 } };              \
        unsigned exp = 1<<n;                                            \
        if(x.u != exp)                                                  \
                panic("expected %x got %x for %s\n", exp,x.u,#field);   \
        assert(x.u == exp);                                             \
} while(0)

#endif
