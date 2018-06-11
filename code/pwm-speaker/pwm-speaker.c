#include "rpi.h"
#include "timer-interrupt.h"
#include "gpio.h"
#include "gpioextra.h"
#include "printf.h"
#include "timer.h"
#include "rpi-stdlib.h"
#include "i2c.h"


#define N 8000
const unsigned trigger = 21,
	      echo = 20,
              speaker = 26,
	      max_dist = N;

static volatile int distance;  	// current measured sonar distance
static volatile unsigned off,	   // led off for this many ticks
		         on, 	   // led on for this manytics,
			 pwm_choices[N], // current pwm choice
			 interrupt_cnt,   // total interrupts 
			 interrupt_period;   // ~usec b/n interrupts

static const unsigned lis3dh_addr = 0x4C; 

// RPI states we have to have memory barriers when switching peripherals.
// no one seems to do this.   currently experimenting.  uncommment this
// if you want to use mem-barriers.
#define mb() do { } while(0)

static unsigned mb_get_time(void) {
	mb();
	unsigned t = timer_get_time();
	mb();
	return t;

}
static unsigned mb_read(int pin) {
	mb();
	unsigned v = gpio_read(pin);
	mb();
	return v;
}

static int timeout_read(int pin, int v, unsigned timeout) {
	unsigned start = mb_get_time();
	while(1) {
		if(mb_read(pin) != v)
			return 1;
		if(mb_get_time() - start > timeout)
			return 0;
	}
}

static void mb_write(int pin, int v) {
	mb();
        gpio_write(pin, v);
	mb();
}

/************************************************************************8888
 * I2C helpers.
 */

static unsigned read_i2c_reg8(unsigned char reg) {
	i2c_write(lis3dh_addr, &reg, 1);
	unsigned char uc = 0;
	i2c_read(lis3dh_addr, &uc, 1);
	return uc;
}

static void write_i2c_reg8(unsigned char reg, unsigned char v) {
	unsigned char data[2];
	data[0] = reg;
	data[1] = v;
	i2c_write(lis3dh_addr, data, 2);
}

static void write_i2c_reg16(unsigned char reg, uint16_t v) {
	unsigned char data[3];
	data[0] = reg;
	data[1] = (v & 0xFF00) >> 8;
	data[2] = v & 0b11111111;
	i2c_write(lis3dh_addr, data, 3);
}

void int_handler(unsigned pc) {
	/*
	   Can have GPU interrupts even though we just enabled timer: 
	   check for timer, ignore if not.

	   p 114:  bits set in reg 8,9
	   These bits indicates if there are bits set in the pending
	   1/2 registers. The pending 1/2 registers hold ALL interrupts
	   0..63 from the GPU side. Some of these 64 interrupts are
	   also connected to the basic pending register. Any bit set
	   in pending register 1/2 which is NOT connected to the basic
	   pending register causes bit 8 or 9 to set. Status bits 8 and
	   9 should be seen as "There are some interrupts pending which
	   you don't know about. They are in pending register 1 /2."
	 */
	volatile rpi_irq_controller_t *r = RPI_GetIRQController();
	if(r->IRQ_basic_pending & RPI_BASIC_ARM_TIMER_IRQ) {
		// do very little in the interrupt handler: just flip
	 	// led off or on.
	  if(pwm_choices[interrupt_cnt%N]){
			gpio_write(speaker,1);
	  }
	  else {
			gpio_write(speaker,0);
	  }
		interrupt_cnt++;


		static unsigned last_int_time = 0;
		unsigned t = timer_get_time();
		interrupt_period = t - last_int_time;
		last_int_time = t;

        	/* 
		 * Clear the ARM Timer interrupt - it's the only interrupt 
		 * we have enabled, so we want don't have to work out which 
		 * interrupt source caused us to interrupt 
		 */
        	RPI_GetArmTimer()->IRQClear = 1;
	}
}

/*********************************************************************
 * compute pwm using a integer line drawing algorithm.   these integer
 * line drawings come up everywhere: propertional share CPU scheduling,
 * noise minimization, ...
 */
// pwm = a line drawing algorithm.
// https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm
void int_line(unsigned *pwm_choice, unsigned n, int x1, int y1) {
        int pos = 0,
        	x0 = 0,
        	y0 = 0,
        	dx = x1,
        	dy = y1,
        	err = (dx>dy ? dx : -dy)/2, e2;

	assert(n==x1+y1);

        for(;;){
                if (x0==x1 && y0==y1) break;
                e2 = err;
                if (e2 >-dx) { err -= dy; x0++; pwm_choice[pos++] = 1; }
                if (e2 < dy) { err += dx; y0++; pwm_choice[pos++] = 0; }
        }
	assert(pos == n);
}

static void compute_pwm(unsigned d, unsigned verbose_p) {
	if(d > max_dist)
		d = max_dist;

	off = d;
	on = max_dist - off;
	assert(on+off == N);
        int_line((unsigned*)pwm_choices, N,on,off);

	// if you want to see what the on/off decisions are.
	if(verbose_p) {
        	printf("------------------------------------------------\n");
        	printf("n=%d, on=%d, off=%d\n", on+off,on,off);
        	for(int i = 0; i < off+on; i++)
                	printf("\tpwm[%d]=%d\n", i,pwm_choices[i]);
	}
}

void power_up() { // from page 29 of datasheet
  while(read_i2c_reg8(0x06) != 0);
  
  write_i2c_reg8(0x00, 0x04);
  write_i2c_reg8(0x1C, 0x65);
  write_i2c_reg8(0x15, 0x05);

  do {
    write_i2c_reg8(0x04, 0x91);
  } while ((read_i2c_reg8(0x06) & 0b00011111) != 0x10);

  // we skipped the optional reading of the module and firmware ID's

  write_i2c_reg8(0x15, 0x06);
  
  do {
    write_i2c_reg8(0x04, 0x92);
  } while ((read_i2c_reg8(0x07) & 0b00000001) != 0x01 || read_i2c_reg8(0x06) != 0xF8);
  
  write_i2c_reg16(0x0C, 0xE100);  
  write_i2c_reg16(0x0E, 0x10FF);
  write_i2c_reg16(0x20, 0x07D0);
  write_i2c_reg16(0x22, 0x5008);
  write_i2c_reg16(0x24, 0xA041);
  write_i2c_reg16(0x26, 0x45D4);
  
  do {
    write_i2c_reg8(0x04, 0x90);
  } while(read_i2c_reg8(0x06) != 0);
}


unsigned measure() {
  write_i2c_reg8(0x04, 0x81);

  // you're supposed to check that register 0x00 reads 0b10000, but not doing it has no negative effect
  // and just speeds up the measurements
  //while((read_i2c_reg8(0x00) & 0b10000) != 0b10000);

  unsigned distance_tot, distanceL, distanceH;

  distanceL = read_i2c_reg8(0x08) >> 1;
  distanceH = read_i2c_reg8(0x09) & 0b11111;

  distance_tot = (distanceH << 8) | distanceL;

  //printf("Low %x\n", distanceL);
  //printf("High %x\n", distanceH);
  //printf("Total %d\n", distance_tot);


  //ERROR CODES
  // 00 - distance valid
  // 01 near target indication
  // 10 far field
  // 11 general error

  /*
  // Uncomment this if you want to see what the error codes are (if something is wrong)
  unsigned error_code;
  error_code = (read_i2c_reg8(0x09) >> 5) & 0b11;
  printf("error code: %x\n", error_code);
  */
  
  return distance_tot;
}


void takeMeasurement() {
  write_i2c_reg8(0x15, 0x05);
  
  do {
    write_i2c_reg8(0x04, 0x91);
  } while ((read_i2c_reg8(0x06) & 0b00011111) != 0x10);

  write_i2c_reg8(0x15, 0x06);

  do {
    write_i2c_reg8(0x04, 0x92);
  } while ((read_i2c_reg8(0x07) & 0b00000001) != 0x01 || read_i2c_reg8(0x06) != 0xF8);
  
  unsigned distance = 0;
  unsigned last_dist = 0;
  
  while(1) {
    static int loop = 0;
    
    if((distance = measure()) < 0) {
      distance = max_dist;
    }
    compute_pwm(distance,0);

    if(distance != last_dist) {
      printf("loop=%d: interrupt cnt=%d,period=%d: distance=%d, on=%d, off=%d\n", 
	     loop,
	     interrupt_cnt, 
	     interrupt_period,
	     distance,on,off);
      last_dist = distance;
    }
    //delay_ms(150);
    loop++;
  }
  

  do {
    write_i2c_reg8(0x04, 0x90);
  } while (read_i2c_reg8(0x06) != 0);
  
}



void notmain(void) {
	uart_init();
	gpio_init();
	install_int_handlers();
	timer_interrupt_init(0x1);
  
	gpio_set_function(speaker, GPIO_FUNC_OUTPUT);
	gpio_set_function(trigger, GPIO_FUNC_OUTPUT);
	gpio_set_function(echo, GPIO_FUNC_INPUT);
	gpio_set_pulldown(echo);

	enable_cache();
  
	system_enable_interrupts();

	delay_ms(100);   // allow time for device to boot up.
	i2c_init();
	delay_ms(100);   // allow time to settle after init.
	  
	power_up();

	takeMeasurement();
	
	debug("DONE!!!");
}
