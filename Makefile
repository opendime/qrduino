CFLAGS:=-Wall -g -O6
#CC=avr-gcc -mmcu=atmega328p
CC=gcc

all: qrencode

qrduino: qrduino.o qrencode.o

dofbit: dofbit.o qrframe.o

qrencode: qrenc.o qrencode.o qrframe.o

clean:
	rm -rf qrencode *.o qrdunio dofbit

realclean: clean
	rm -rf *~ \#*

