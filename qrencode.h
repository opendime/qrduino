#include "framask.h"

unsigned char strinbuf[(DATAWID + ECCWID) * (BLOCKS1 + BLOCKS2) + BLOCKS2];

// greater of 600 or WDB*WD
#define FBSIZ WDB*WD
#if FBSIZ < 800
#undef FBSIZ
#define FBSIZ 800
#endif

unsigned char qrframe[FBSIZ];
#define QRBIT(x,y) ( ( qrframe[((x)>>3) + (y) * WDB] >> (7-((x) & 7 ))) & 1 )
#define SETQRBIT(x,y) qrframe[((x)>>3) + (y) * WDB] |= 0x80 >> ((x) & 7)
#define TOGQRBIT(x,y) qrframe[((x)>>3) + (y) * WDB] ^= 0x80 >> ((x) & 7)
#define FIXEDBIT(x,y) ( ( __LPM(&framask[((x)>>3) + (y) * WDB]) >> (7-((x) & 7 ))) & 1 )

void qrencode(void);
