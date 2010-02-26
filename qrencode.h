#ifndef __AVR__
#define PROGMEM
#define memcpy_P memcpy
#define __LPM(x) *x
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

//qrframe only
unsigned initecc(unsigned char level,unsigned char version);
 void initframe(void);

void qrencode(void);
