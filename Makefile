
.PHONY: all
all:
	avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o main.o main.c
	avr-gcc -mmcu=atmega328p main.o -o blinky
	avr-objcopy -O ihex -R .eeprom blinky blinky.hex

.PHONY: upload
upload:
	avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyACM0 -b 115200 -U flash:w:blinky.hex
