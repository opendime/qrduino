#ifndef __AVR__
#define PROGMEM
#define memcpy_P memcpy
#define __LPM(x) *x
#define pgm_read_word(x) *x
#else
#include <avr/pgmspace.h>
#define USEPRECALC
#endif

extern unsigned char  WD, WDB;
#ifndef USEPRECALC
extern unsigned char *strinbuf;
extern unsigned char *qrframe;
#else
extern unsigned char strinbuf[];
extern unsigned char qrframe[];
#endif

#include "qrbits.h"

void qrencode(void);

//qrframe only
void initframe(void);
unsigned initeccsize(unsigned char ecc, unsigned char size);
unsigned initecc(unsigned char level,unsigned char version);

