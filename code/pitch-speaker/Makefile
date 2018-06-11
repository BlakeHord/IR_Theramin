NAME = pitch-speaker
LIBPI = ../../libpi
CC= arm-none-eabi-gcc
CFLAGS = -I$(LIBPI) -I. -Wall -O2 -std=gnu99 -nostdlib -nostartfiles -ffreestanding 

SRC = $(NAME).c 
OBJS = $(SRC:.c=.o)

all : kernel.img

kernel.img : $(LIBPI)/memmap $(LIBPI)/start.o $(OBJS)
	make -C $(LIBPI)
	arm-none-eabi-ld  $(LIBPI)/start.o $(OBJS) $(LIBPI)/libpi.a -T $(LIBPI)/memmap -o $(NAME).elf
	arm-none-eabi-objdump -D $(NAME).elf > $(NAME).list
	arm-none-eabi-objcopy $(NAME).elf -O binary kernel.img

clean :
	rm -f *.o *.bin *.elf *.list *.img *~ Makefile.bak

depend:
	makedepend -I$(LIBPI) *.[ch] 

# DO NOT DELETE

sonar.o: ../../libpi/rpi.h ../../libpi/printf.h ../../libpi/uart.h
sonar.o: ../../libpi/kalloc.h ../../libpi/timer.h ../../libpi/gpio.h
sonar.o: ../../libpi/gpioextra.h
