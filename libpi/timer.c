#include "rpi.h"

void delay(unsigned ticks) {
        while(ticks-- > 0)
                asm("add r1, r1, #0");
}

unsigned timer_get_time(void) {
	return *(volatile unsigned *)0x20003004;
}

void delay_us(unsigned us) {
    unsigned rb = timer_get_time();
    while (1) {
        unsigned ra = timer_get_time();
        if ((ra - rb) >= us) {
            break;
        }
    }
}

void delay_ms(unsigned ms) {
	delay_us(ms*1000);
}

// ticks = clock cycles.  we over-delay currently.
void delay_cycles(unsigned ticks) {
        while(ticks-- > 0)
                asm("add r1, r1, #0");
}

// paranoid barriers since they are likely using different periphs.   easily
// deleted or make a delay_us_map_raw() version.
void delay_us_map(unsigned us, void (*fn)(void)) {
	mem_barrier();
	unsigned start = timer_get_time();
	while((timer_get_time() - start) < us) { 
		// could put a check here that it completes within some usec
		// budget
		mem_barrier();
			fn();
		mem_barrier();
    	}
	mem_barrier();
}

void delay_ms_map(unsigned ms, void (*fn)(void)) {
	delay_us_map(ms*1000, fn);
}

