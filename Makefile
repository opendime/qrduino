CFLAGS:=-Wall -g -Os
#CC=avr-gcc -mmcu=atmega328p
CC=gcc

all: dofbit qrencode

qrduino: qrduino.o qrencode.o

dofbit: dofbit.o

qrenc.o qrencode.o: qrencode.h framask.h

qrencode: qrenc.o qrencode.o

clean:
	rm -rf qrencode *.o qrdunio dofbit

realclean: clean
	rm -rf *~ \#*

v6L:
	./dofbit 6 1 >framask.h
	make qrencode

v7L:
	./dofbit 7 1 >framask.h
	make qrencode
