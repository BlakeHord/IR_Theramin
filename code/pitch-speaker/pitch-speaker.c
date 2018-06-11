#include "rpi.h"
#include "gpio.h"
#include "gpioextra.h"
#include "printf.h"
#include "timer.h"
#include "rpi-stdlib.h"
#include "i2c.h"


const unsigned trigger = 21,
	      echo = 20,
              speaker = 26;

static volatile int distance;  	// current measured sonar distance
static const unsigned lis3dh_addr = 0x4C; 

void playPeriod(unsigned period, int durationMs)
{
    //Calculate the period in microseconds
    //int periodMicro = (1000000/freqHz);
    int halfPeriod = period/2;
     
    //store start time
    int startTime = timer_get_time();
     
    //(millis() - startTime) is elapsed play time
    while ((timer_get_time() - startTime) < durationMs)    
    {
        gpio_write(speaker, 1);
        delay_us(halfPeriod);
        gpio_write(speaker, 0);
        delay_us(halfPeriod);
    }
  
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

  while(1) {
    distance = measure();

    playPeriod(distance, distance * 15);
    //playPeriod(distance/2 ,10000); 
  }

  do {
    write_i2c_reg8(0x04, 0x90);
  } while (read_i2c_reg8(0x06) != 0);
}



void notmain(void) {
	uart_init();
	gpio_init();
  
	gpio_set_function(speaker, GPIO_FUNC_OUTPUT);
	gpio_set_function(trigger, GPIO_FUNC_OUTPUT);
	gpio_set_function(echo, GPIO_FUNC_INPUT);
	gpio_set_pulldown(echo);

	enable_cache();
  
	delay_ms(100);   // allow time for device to boot up.
	i2c_init();
	delay_ms(100);   // allow time to settle after init.

	/*
	// a scale to test the speaker at
	unsigned scale[13] = {2270, 2100, 2000, 1900, 1800, 1700, 1600, 1500, 1400, 1350, 1280, 1200, 1140};
	
	for (int i = 0; i < 13; i++) {
	  // plays scale - each note half a second
	  playPeriod(scale[i], 500000);
	}
	*/
	
	/*
	// This is just a cool noise if you play it
	for (unsigned i = 10; i < 8000; i = (i*11)/10) {
	  playPeriod(i, 10000);
	}
	*/
	
	power_up();

	takeMeasurement();
	
	debug("DONE!!!");
}
