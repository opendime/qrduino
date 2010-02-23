CFLAGS:=-Wall -g -Os
#CC=avr-gcc -mmcu=atmega328p
CC=gcc

all: qrencode dofbit

qrduino: qrduino.o qrencode.o

dofbit: dofbit.o

qrenc.o qrencode.o: qrencode.h framask.h

qrencode: qrenc.o qrencode.o

clean:
	rm -rf qrencode *.o qrdunio
