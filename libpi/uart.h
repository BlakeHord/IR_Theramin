#ifndef UART_DEFS
#define UART_DEFS

void uart_init ( void );
int uart_getc ( void );
void uart_putc ( unsigned int c );
void uart_flush ( void );
int uart_rx_has_data();
void uart_disable(void);

#endif
