// modified version of pat's code.  i put some barriers, also volatiled it up.
#include "gpio.h"
#include "gpioextra.h"
#include "uart.h"
#include "mem-barrier.h"

struct UART {
    volatile unsigned data; // I/O Data
    volatile unsigned ier;  // Interrupt enable
    volatile unsigned iir;  // Interrupt identify and fifo enables/clears
    volatile unsigned lcr;  // line control register
    volatile unsigned mcr;  // modem control register
    volatile unsigned lsr;  // line status register
    volatile unsigned msr;  // modem status register
    volatile unsigned scratch;
    volatile unsigned cntl; // control register
    volatile unsigned stat; // status register
    volatile unsigned baud; // baud rate register
} ;

// AUX bits
#define AUX_ENABLES 0x20215004
#define AUX_ENABLE  0x00000001

// Mini UART
#define MINI_UART_BASE 0x20215040

#define MINI_UART_IIR_RX_FIFO_CLEAR  0x00000002
#define MINI_UART_IIR_TX_FIFO_CLEAR  0x00000004
#define MINI_UART_IIR_RX_FIFO_ENABLE 0x00000008
#define MINI_UART_IIR_TX_FIFO_ENABLE 0x00000004

#define MINI_UART_LCR_8BIT       0x00000003

#define MINI_UART_LSR_RX_READY   0x00000001
#define MINI_UART_LSR_TX_READY   0x00000010
#define MINI_UART_LSR_TX_EMPTY   0x00000020

#define MINI_UART_CNTL_TX_ENABLE 0x00000002
#define MINI_UART_CNTL_RX_ENABLE 0x00000001

static volatile struct UART *uart = (struct UART*) MINI_UART_BASE;

/* Key detail from the Broadcom Peripherals data sheet p.10
 *
 * GPIO pins should be set up first the before enabling the UART. 
 * The UART core is build to emulate 16550 behaviour. 
 * So when it is enabled any data at the inputs will immediately be received .
 * If the UART1_RX line is low (because the GPIO pins have not been set-up yet) 
 * that will be seen as a start bit and the UART will start receiving 0x00-characters.
 * [...] The result will be that the FIFO is full and overflowing in no time flat.
 */

static int disable_p = 0;
void uart_disable(void) {
	disable_p = 1;
}

void uart_init(void) {
    mem_barrier();

    gpio_set_function(GPIO_TX, GPIO_FUNC_ALT5);
    gpio_set_function(GPIO_RX, GPIO_FUNC_ALT5);
#if 0
    // dwelch does this.  not sure if we need to.
    gpio_set_pud(GPIO_PIN14, 0);
    gpio_set_pud(GPIO_PIN15, 0);
#endif

    mem_barrier();

    volatile unsigned *aux = (void*)AUX_ENABLES;
    // is |= correct?
    *aux |= AUX_ENABLE; // turn on mini-uart

    // this took me a while to figure out.  must have a memory barrier.
    mem_barrier();

    uart->ier  = 0;
    uart->cntl = 0;
    uart->lcr  = MINI_UART_LCR_8BIT;
    uart->mcr  = 0;
    uart->ier  = 0;
    uart->iir  = MINI_UART_IIR_RX_FIFO_CLEAR |
                 MINI_UART_IIR_RX_FIFO_ENABLE |
                 MINI_UART_IIR_TX_FIFO_CLEAR |
                 MINI_UART_IIR_TX_FIFO_ENABLE;

    uart->baud =  270; // baud rate ((250,000,000/115200)/8)-1 = 270
    // uart->baud =  3254; // baud rate (250000000/9600)/8-1 = 3254


    uart->cntl = MINI_UART_CNTL_TX_ENABLE | MINI_UART_CNTL_RX_ENABLE;;
}

int uart_getc(void) {
	if(disable_p)
		return 0;
    	while (!(uart->lsr & MINI_UART_LSR_RX_READY)) ;
    	return uart->data & 0xFF;
}

int uart_rx_has_data() {
	return (uart->lsr & MINI_UART_LSR_RX_READY) != 0;
}

int uart_getc_async(unsigned char *c) {
	if(disable_p)
		return 0;
	
	if((uart->lsr & MINI_UART_LSR_RX_READY)) {
		*c = uart->data & 0xFF;
		return 1;
	}
	return 0;
}

void uart_putc(unsigned c) {
	if(disable_p)
		return;
#if 0
    if (c == '\n') {
        uart_putc('\r');
    }
#endif
	while (!(uart->lsr & MINI_UART_LSR_TX_EMPTY))
		;
    	uart->data = c;
}

void uart_flush(void) {
	if(disable_p)
		return;
    	while (!(uart->lsr & MINI_UART_LSR_TX_EMPTY)) ;
}
