
#if 1 // Version 6L
#define DATAWID (68)
#define ECCWID (18)
#else // Version 7L
#define DATAWID (78)
#define ECCWID (20)
#endif

unsigned char strinbuf[(DATAWID+ECCWID)*2];

#include "framask.h"

// greater of 600 or WDB*WD
#define FBSIZ WDB*WD
#if FBSIZ < 600
#undef FBSIZ
#define FBSIZ 600
#endif

unsigned char qrframe[FBSIZ];
#define QRBIT(x,y) ( ( qrframe[((x)>>3) + (y) * WDB] >> (7-((x) & 7 ))) & 1 )
#define SETQRBIT(x,y) qrframe[((x)>>3) + (y) * WDB] |= 0x80 >> ((x) & 7)
#define TOGQRBIT(x,y) qrframe[((x)>>3) + (y) * WDB] ^= 0x80 >> ((x) & 7)
#define FIXEDBIT(x,y) ( ( __LPM(&framask[((x)>>3) + (y) * WDB]) >> (7-((x) & 7 ))) & 1 )

void qrencode(void);
