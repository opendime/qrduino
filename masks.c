#include <stdio.h>
#include "framev6.h"

main() {
    unsigned char *b;
    unsigned x,y;
    unsigned char bits;

    printf( "unsigned char frame[] = {\n" );
    b = baseframe;
    for( y = 0 ; y< 41; y++ ) {
        for( x = 0 ; x< 41; x++ ){
            bits <<= 1;
            bits |= *b++ & 1;
            if( (x & 7) == 7 )
                printf( "%02x,", bits);
        }
        printf( "%02x,\n", (bits&1) << 7 );
    }
    printf( "};\n" );


    printf( "unsigned char mask[] = {\n" );
    b = baseframe;
    for( y = 0 ; y< 41; y++ ) {
        for( x = 0 ; x< 41; x++ ){
            bits <<= 1;
            bits |= !!(*b++ & 0x80);
            if( (x & 7) == 7 )
                printf( "%02x,", bits);
        }
        printf( "%02x,\n", (bits&1) << 7 );
    }
    printf( "};\n" );
}
