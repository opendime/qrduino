CFLAGS:=-Wall -g -Os
#CC=avr-gcc -mmcu=atmega328p
CC=gcc

qrencode: qrenc.o qrencode.o

qrduino: qrduino.o qrencode.o

clean:
	rm -rf qrencode *.o qrdunio
