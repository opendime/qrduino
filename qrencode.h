#define WD (41)
#define CL (172)
unsigned char qrframe[600];
unsigned char strinbuf[CL];
#define WDB ((WD+7)/8)
#define QRBIT(x,y) ( ( qrframe[((x)>>3) + (y) * WDB] >> (7-((x) & 7 ))) & 1 )
#define SETQRBIT(x,y) qrframe[((x)>>3) + (y) * WDB] |= 0x80 >> ((x) & 7)
#define TOGQRBIT(x,y) qrframe[((x)>>3) + (y) * WDB] ^= 0x80 >> ((x) & 7)
#define FIXEDBIT(x,y) ( ( framask[((x)>>3) + (y) * WDB] >> (7-((x) & 7 ))) & 1 )

void qrencode(void);

