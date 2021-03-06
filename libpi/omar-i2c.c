// do we need memory barriers here?
#include "gpio.h"
#include "rpi.h"
#include "i2c.h"

// typedef unsigned uint32_t;

struct I2C { // I2C registers
    uint32_t control;
    uint32_t status;
    uint32_t data_length;
    uint32_t slave_address;
    uint32_t data_fifo;
    uint32_t clock_divider;
    uint32_t data_delay;
    uint32_t clock_stretch_timeout;
};

#define	CONTROL_READ				0x0001
#define CONTROL_CLEAR_FIFO	0x0010
#define CONTROL_START				0x0080
#define CONTROL_ENABLE			0x8000

#define STATUS_TRANSFER_ACTIVE	0x001
#define STATUS_TRANSFER_DONE		0x002
#define STATUS_FIFO_NEED_WRITE	0x004
#define STATUS_FIFO_NEED_READ		0x008
#define STATUS_FIFO_CAN_WRITE		0x010
#define STATUS_FIFO_CAN_READ		0x020
#define STATUS_FIFO_EMPTY				0x040
#define STATUS_FIFO_FULL				0x080
#define STATUS_ERROR_SLAVE_ACK	0x100
#define STATUS_TIMEOUT					0x200

#define FIFO_MAX_SIZE 16

/*
 * There are three I2C masters on the Raspberry Pi, with the following
 * virtual memory addresses:
 * 	• BSC0: 0x7E20_5000
 * 	• BSC1: 0x7E80_4000
 * 	• BSC2: 0x7E80_5000
 * We are using BSC1. The other two do not have available GPIO pins
 * for use on the A+ model.
*/

#define SDA GPIO_PIN2
#define SCL GPIO_PIN3
#define BSC_BASE 0x20804000

static volatile struct I2C *i2c = (struct I2C *) BSC_BASE;

// #define SHORT_DELAY 50
#define NORM_DELAY 500

void i2c_init(void) {
    gpio_set_function(SDA, GPIO_FUNC_ALT0);
    gpio_set_function(SCL, GPIO_FUNC_ALT0);
    i2c->control = CONTROL_ENABLE;
}

int i2c_read(unsigned slave_address, uint8_t data[], unsigned data_length) {
    // clear out the FIFO
    i2c->control |= CONTROL_CLEAR_FIFO;
    while (!(i2c->status & STATUS_FIFO_EMPTY))
        ;
    // clear previous transfer's flags
    // printf("status before = %x\n", i2c->status);
    i2c->status |= STATUS_TRANSFER_DONE |
                   STATUS_ERROR_SLAVE_ACK |
                   STATUS_TIMEOUT;
    // printf("status after = %x\n", i2c->status);

    // set slave address + data length
    i2c->slave_address = slave_address;
    i2c->data_length = data_length;
    int data_index = 0;

    // begin read
    i2c->control |= CONTROL_READ | CONTROL_START;

    delay_us(NORM_DELAY);
    // keep reading until transfer is complete
    while ((i2c->status & STATUS_FIFO_CAN_READ) &&
            (!(i2c->status & STATUS_TRANSFER_DONE) ||
             (data_index < data_length))) {
        data[data_index++] = i2c->data_fifo;
    }

    // inform end user of potential responses
    if (i2c->status & STATUS_ERROR_SLAVE_ACK) {
        printf("NACK received.\n");
    }
    if (i2c->status & STATUS_TIMEOUT) {
        printf("Clock timed out.\n");
    }
    if (data_index < data_length) {
        printf("Data transfer incomplete.\n");
    }
	return 1;
}

int i2c_write(unsigned slave_address, uint8_t data[], unsigned data_length) {
    // clear out the FIFO
    i2c->control |= CONTROL_CLEAR_FIFO;
    while (!(i2c->status & STATUS_FIFO_EMPTY))
	;
    // clear previous transfer's flags
    i2c->status |= STATUS_TRANSFER_DONE |
                   STATUS_ERROR_SLAVE_ACK |
                   STATUS_TIMEOUT;

    // set slave address + data length
    i2c->slave_address = slave_address;
    i2c->data_length = data_length;
    int data_index = 0;

    // write first 16 chunks into FIFO
    while ((data_index < FIFO_MAX_SIZE) &&
            (data_index < data_length)) {
        i2c->data_fifo = data[data_index++];
	
    }

    // begin write
    i2c->control &= ~CONTROL_READ;
    i2c->control |= CONTROL_START;

    // as fifo clears up, continue transferring until done
    while (!(i2c->status & STATUS_TRANSFER_DONE) &&
            (i2c->status & STATUS_FIFO_CAN_WRITE) &&
            (data_index < data_length)) {
        i2c->data_fifo = data[data_index++];
    }

    // wait until the FIFO's contents are emptied by the slave
    while (!(i2c->status & STATUS_FIFO_EMPTY)) {
    	// inform end user of potential responses
    	if (i2c->status & STATUS_ERROR_SLAVE_ACK) {
        	printf("i2c_write: NACK received.\n");
		return 0;
    	}
    	if (i2c->status & STATUS_TIMEOUT) {
        	printf("i2c_write: Clock timed out.\n");
		return 0;
    	}
        // this is broken, right?  data_index is not touched.
    	if (data_index < data_length) {
        	printf("i2c_write: Data transfer incomplete.\n");
    	}
    }
	return 1;
}
