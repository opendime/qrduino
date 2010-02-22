#include <avr/io.h>
#include <util/delay.h>
void LcdWrite(unsigned char dc, unsigned char data)
{
    int i;
    if( dc )
        PORTD |= 0x20;
    else
        PORTD &= ~0x20;
    PORTD &= ~0x80;
     for( i = 0 ; i < 8 ; i++ ) {
        if( (data >> (7 - i)) & 1 )
            PORTD |= 16;
        else
            PORTD &= ~16;
        PORTD |= 8;
        PORTD &= ~8;
     }
     PORTD |= 0x80;
     PORTB ^= 0x20;
}


#define WD (41)
#define CL (172)

extern unsigned char qrframe[WD * WD];
extern unsigned char strinbuf[CL];
extern void qrcode(void);
#include <string.h>
main()
{
    DDRB |= 0x20;
    PORTB |= 0xf8;

    DDRD |= 0xf8;
    PORTD &= ~8;
    PORTD &= ~0x40;
    PORTD |= 0x40;

    LcdWrite(0, 0x22);
    LcdWrite(0, 0x0C);

    unsigned i, j, k, t;
    unsigned char b, *c;
    strcpy((char *)strinbuf, "Hello World!");
    qrcode();
    PORTB &= ~0x20;
    t = 0;
    b = 0;
    for ( k = 0; k < 84 ; k++) {
        for ( i = 0; i < 48 ; i++) {
            b >>= 1;
            if( i < WD && k < WD )
                if( qrframe[k*WD+40-i] & 1)
                    b |= 0x80;
            if( ++t > 7 ) {
                t = 0;
                LcdWrite(1, b);
                b = 0;
            }
        }
    }
    return 0;
}
