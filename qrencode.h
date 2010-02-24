#include "framask.h"

unsigned char strinbuf[(DATAWID + ECCWID) * (BLOCKS1 + BLOCKS2) + BLOCKS2];

// greater of 600 or WDB*WD
#define FBSIZ WDB*WD
#if FBSIZ < 800
#undef FBSIZ
#define FBSIZ 800
#endif

unsigned char qrframe[FBSIZ];

#include "qrbits.h"

void qrencode(void);
