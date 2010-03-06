#include <stdio.h>
#include <string.h>

unsigned char f[4000][4000];
unsigned runs[8];
unsigned w, h;
unsigned modwid;
#define MAXCANDS 32
unsigned cands = 1;
unsigned fx[MAXCANDS];
unsigned fy[MAXCANDS];
unsigned fh[MAXCANDS];
unsigned fw[MAXCANDS];

unsigned iabs(int a)
{
    if (a < 0)
        return -a;
    return a;
}

#if 0
int despeckle()
{
    int x, y, max = 6;

    for (x = 1; x < max; x++) {
        if (runs[x] != 1)
            continue;
        if (runs[x - 1] == 1)
            continue;
        if (runs[x + 1] == 1)
            continue;
        runs[x - 1] += 1 + runs[x + 1];
        max -= 2;
        for (y = x; y < max - 1; y++)
            runs[y] = runs[y + 2];
        x--;
    }
    return 6 - max;
}
#endif

int isfinder2()
{
    int a, b, c, d, e, m;
    a = runs[1];
    e = runs[5];
    if (iabs(a - e) > (a + e + 1) / 4)
        return 0;
    b = runs[2];
    d = runs[4];
    if (iabs(b - d) > (b + d + 1) / 4)
        return 0;
    if( (a+e)*2 < (b+d) )
        return 0;
    if( (b+d)*2 < (a+e) )
        return 0;
    c = runs[3];
    if( modwid ) {
        if( c * 14 < modwid * 5 )
            return 0;
        if( c * 10 > modwid * 7 )
            return 0;
    }
    m = a + e + (b + d + 1) / 2;
    if (iabs(c - m) > (c + m) / 4)
        return 0;
    return 1;
}

// line 7, 1 pixel per module would be a special case I don't handle
unsigned long ave = 0;
unsigned long avex, avey;

int center(unsigned char j, unsigned lave)
{
    // use u d l r
    unsigned r, v, x, y;
    x = fx[j];
    y = fy[j];

    fprintf( stderr, "C %d %d %d %d\n", x, y, f[y][x], lave );

    for (r = x - 1; r > 0; r--)
        if (f[y][r] > lave)
            break;
#if 0
    for (; r > 0; r--)
        if (f[y][r] < lave)
            break;
#endif
    r++;
    for (v = x + 1; v < w; v++)
        if (f[y][v] > lave)
            break;
#if 0
    for (; v < w; v++)
        if (f[y][v] < lave)
            break;
#endif
    v--;
    x = (r + v) / 2;
    fx[j] = x;
    fw[j] = v - r;
    for (r = y - 1; r > 0; r--)
        if (f[r][x] > lave)
            break;
#if 0
    for (; r > 0; r--)
        if (f[r][x] < lave)
            break;
#endif
    r++;
    for (v = y + 1; v < h; v++)
        if (f[v][x] > lave)
            break;
#if 0
    for (; v < h; v++)
        if (f[v][x] < lave)
            break;
#endif
    v--;
    y = (r + v) / 2;
    fy[j] = y;
    fh[j] = v - r;
    return fw[j] + fh[j];
}

int findit()
{
    int x = 0, y, i, b, r, v, found;
    found = 0;
    // 13 for 2 pixel min, do line 7 if simple 1 ppmod
    for (y = 13; !found && y < h && y < w; y += 2) {
        ave = 0;
        for (x = 0; x <= y; x++)
            ave += f[y - x][x];
        ave += y / 2;
        ave /= y + 1;
        b = 0, r = 0, i = 0;

        // Note that we only need the current 5 runs, not a  list
        runs[i] = 0;
        for (x = 0; x <= y; x++) {
            v = b;
            if (f[y - x][x] < ave - 8)
                v = 1;
            if (f[y - x][x] > ave + 8)
                v = 0;
            if (v == b) {
                r++;
                continue;
            }
            b = v;
            runs[i++] = r;
            runs[i] = 0;
            r = 1;

        if ( i > 6 ) {
            for( v = 0 ; v < 6; v++ )
                runs[v] = runs[v+2];
            i -= 2;
        }
        if (i > 5) {
            int xx;
            fprintf(stderr, "%d,%d,%d,%d,%d - %d, %d\n", runs[1],runs[2],runs[3],runs[4],runs[5], i, x);
            if (!isfinder2())
                continue;
            xx = x - runs[5] - runs[4] - (runs[3]/2);
            fx[0] = xx;
            fy[0] = y - xx;
            modwid = center(0, ave);
            
            fprintf(stderr, " o %d,%d %d %d\n", fx[0], fy[0], fw[0], fh[0]);

            y -= x;
            found = 1;
        }
        }
    }
    return found;
}

// maybe add ave from origin
void findnexty(unsigned char t)
{
    int x = 0, y, i, b, r, v;
    avey = ave * 64;
    x = fx[t];
    b = 0, r = 0, i = 0;
    runs[i] = 0;
    fprintf(stderr, "x=%d y=%d,", x, fy[t]);
    for (y = fy[t]; y < h; y++) {
        avey += f[y][x];
        avey -= avey / 64;

        v = f[y][x] <= avey / 64;
        if (v == b) {
            r++;
            if( y+1 != h )
                continue;
        }

        b = v;

        runs[i++] = r;
        runs[i] = 0;
        r = 1;

        if ( i > 6 ) {
            for( v = 0 ; v < 6; v++ )
                runs[v] = runs[v+2];
            i -= 2;
        }
        if (i > 5) {
            int xx;
            fprintf(stderr, "%d,%d,%d,%d,%d - %d, %d\n", runs[1],runs[2],runs[3],runs[4],runs[5], i, x);

            if (runs[1] * 8 < modwid || runs[1] * 4 > modwid)
                continue;
            if (!isfinder2())
                continue;
            xx = y - runs[5] - runs[4] - (runs[3]/2);

            fprintf(stderr, "Y: %d\n", xx);
            fy[cands] = xx;
            fx[cands] = fx[t];
            center(cands, avey/64);

            fprintf(stderr, " + %d,%d %d %d\n", fx[cands], fy[cands], fw[cands], fh[cands]);
            
            if (fw[cands] * 3 < modwid || fw[cands] > modwid)
                continue;
            if (fh[cands] * 3 < modwid || fh[cands] > modwid)
                continue;
            //        f[fy[cands]][fx[cands]] = 255;
            cands++;
        }
    }
    fprintf(stderr, "\n");
    return;
}

void findnextx(unsigned char t)
{
    int x = 0, y, i, b, r, v;
    avex = ave * 64;
    y = fy[t];
    b = 0, r = 0, i = 0;
    runs[i] = 0;

    fprintf(stderr, "y=%d x=%d,", y, fx[t]);

    for (x = fx[t]; x < w; x++) {
        avex += f[y][x];
        avex -= avex / 64;

        v = f[y][x] <= avex / 64;
        if (v == b) {
            r++;
            if( x+1 != w )
                continue;
        }

        b = v;
        runs[i++] = r;
        runs[i] = 0;
        r = 1;
        if ( i > 6 ) {
            for( v = 0 ; v < 6; v++ )
                runs[v] = runs[v+2];
            i -= 2;
        }
        if (i > 5) {
            int xx;
            fprintf(stderr, "%d,%d,%d,%d,%d - %d, %d a %d\n", runs[1],runs[2],runs[3],runs[4],runs[5], i, x, avex/64);

            if (runs[1] * 8 < modwid || runs[1] * 4 > modwid)
                continue;
            if (!isfinder2())
                continue;
            xx = x - runs[5] - runs[4] - (runs[3]/2);
            fprintf(stderr, "X: %d\n", xx );
            fx[cands] = xx;
            fy[cands] = fy[t];
            center(cands, avex/64);
            
            fprintf(stderr, " + %d,%d %d %d\n", fx[cands], fy[cands], fw[cands], fh[cands]);
            
            if (fw[cands] * 3 < modwid || fw[cands] > modwid)
                continue;
            if (fh[cands] * 3 < modwid || fh[cands] > modwid)
                continue;

            cands++;
        }
    }
    fprintf(stderr, "\n");
    return;
}

void readgray()
{
    char buf[512];
    unsigned x, y, m, s;
    scanf("%2s", buf);
    scanf("%u %u", &w, &h);
    scanf("%u", &s);
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) {
            scanf("%u", &m);
            f[y][x] = m;
        }
}

main()
{
    int i, j;
    readgray();

    findit();

    findnexty(0);
    findnextx(0);

    j = cands;
    i = 1;
    while (i < j) {
        findnexty(i);
        findnextx(i);
        i++;
    }
    i = j + 1;
    j = cands;
    while (i < j) {
        findnexty(i);
        findnextx(i);
        i++;
    }

    for (i = 0; i < cands - 1; i++)
        for (j = i + 1; j < cands; j++)
            if (fx[i] == fx[j] && fy[i] == fy[j]
              && fw[i] == fw[j] && fh[i] == fh[j]) {
        fprintf(stderr, "DUP - %d,%d %d %d\n", fx[i], fy[i], fw[i], fh[i]);
                if (j < cands - 1) {
                    fx[j] = fx[cands - 1];
                    fy[j] = fy[cands - 1];
                    fw[j] = fw[cands - 1];
                    fh[j] = fh[cands - 1];
                    j--;
                }
                cands--;
            }

    for (i = 0; i < cands; i++) {
        fprintf(stderr, "%d,%d %d %d\n", fx[i], fy[i], fw[i], fh[i]);
        f[fy[i]][fx[i]] = 255;
    }
    fprintf(stderr, "\n");

    if( i < 3 )
        return;

    printf("P2\n%u %u\n255\n", w, h);
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            printf("%u ", f[j][i]);

}
