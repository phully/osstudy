all:
	arm-eabi-as -mcpu=arm926ej-s startup.s -g -o startup.o
	arm-eabi-gcc -nostdlib -nostartfiles -ffreestanding -c -mcpu=arm926ej-s -g main.c -o main.o
	arm-eabi-ld -T startup.ld main.o startup.o -o loader.elf
	arm-eabi-objcopy -S -O binary loader.elf loader.bin