#ifndef __RPI_H__
#define __RPI_H__

void PUT32(unsigned addr,unsigned v);
unsigned GET32(unsigned addr);

void delay_us(unsigned us);
void delay_ms(unsigned ms);
void delay_cycles(unsigned ticks);
void delay_us_map(unsigned us, void (*fn)(void));
void delay_ms_map(unsigned ms, void (*fn)(void));

/* libm.c */
#define M_PI 3.1415926536897932384626

float floor(float f);
float fabs(float f);
float sin(float x);
float cos(float x);

int strcmp(const char *a, const char *b);
void *memset(void *_p, int c, unsigned n);

void enable_cache(void) ;
void disable_cache(void) ;
void dummy(unsigned);

void PUT32(unsigned addr, unsigned v );
void PUT16(unsigned addr, unsigned v );
void PUT8(unsigned addr, unsigned v );
unsigned GET32( unsigned int );
unsigned GETPC(void);

void reboot(void);

void enable_fp(void);


#include "printf.h"
#include "uart.h"
#include "kalloc.h"
#include "timer.h"
#include "mem-barrier.h"

typedef unsigned uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef int int32_t;
typedef signed char int8_t;

#define offsetof(st, m) __builtin_offsetof(st, m)

#endif

