#define QRBIT(x,y) ( ( qrframe[((x)>>3) + (y) * qrwidbytes] >> (7-((x) & 7 ))) & 1 )
#define SETQRBIT(x,y) qrframe[((x)>>3) + (y) * qrwidbytes] |= 0x80 >> ((x) & 7)
#define TOGQRBIT(x,y) qrframe[((x)>>3) + (y) * qrwidbytes] ^= 0x80 >> ((x) & 7)

#define SETFXBIT(x,y) framask[((x)>>3) + (y) * qrwidbytes] |= 0x80 >> ((x) & 7)
#define FIXEDBIT(x,y) ( ( framask[((x)>>3) + (y) * qrwidbytes] >> (7-((x) & 7 ))) & 1 )
