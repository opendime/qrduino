extern unsigned char  WD, WDB;
extern unsigned char *strinbuf;
extern unsigned char *qrframe;

#include "qrbits.h"

//qrframe only
unsigned initecc(unsigned char level,unsigned char version);
 void initframe(void);

void qrencode(void);
