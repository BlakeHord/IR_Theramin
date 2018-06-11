#ifndef GPIOEXTRA_H
#define GPIOEXTRA_H

enum {
    GPIO_PUD_DISABLE  = 0,
    GPIO_PUD_PULLDOWN = 1,
    GPIO_PUD_PULLUP   = 2,
};
void gpio_set_pud(unsigned pin, unsigned pud);
void gpio_set_pullup(unsigned pin);
void gpio_set_pulldown(unsigned pin);

#endif
