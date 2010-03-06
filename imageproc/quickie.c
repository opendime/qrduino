#include <stdio.h>
#include <string.h>

unsigned char f[4000][4000];
unsigned runs[4000];
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

int despeckle(int max)
{
    int x, y;

    for (x = 1; x < max - 1; x++) {
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
    return max;
}

int isfinder(int x)
{
    int a, b, c, d, e, m;
    a = runs[x];
    e = runs[x + 4];
    if (iabs(a - e) > (a + e) / 4)
        return 0;
    b = runs[x + 1];
    d = runs[x + 3];
    if (iabs(b - d) > (b + d) / 4)
        return 0;
    c = runs[x + 2];
    m = a + e + (b + d) / 2;
    if (iabs(c - m) > (c + m) / 4)
        return 0;
    return 1;
}

// line 7, 1 pixel per module would be a special case I don't handle
unsigned long ave = 0;
unsigned long avex, avey;

int center(unsigned char j)
{
    unsigned r, v, x, y;
    x = fx[j];
    y = fy[j];

    for (r = x - 1; r > 0; r--)
        if (f[y][r] > ave)
            break;
#if 0
    for (; r > 0; r--)
        if (f[y][r] < ave)
            break;
#endif
    r++;
    for (v = x + 1; v < h; v++)
        if (f[y][v] > ave)
            break;
#if 0
    for (; v < h; v++)
        if (f[y][v] < ave)
            break;
#endif
    v--;
    x = (r + v) / 2;
    fx[j] = x;
    fw[j] = v - r;
    for (r = y - 1; r > 0; r--)
        if (f[r][x] > ave)
            break;
#if 0
    for (; r > 0; r--)
        if (f[r][x] < ave)
            break;
#endif
    r++;
    for (v = y + 1; v < h; v++)
        if (f[v][x] > ave)
            break;
#if 0
    for (; v < h; v++)
        if (f[v][x] < ave)
            break;
#endif
    v--;
    y = (r + v) / 2;
    fy[j] = y;
    fh[j] = v - r;
    return fw[j]+fh[j];
}

int findit()
{
    int x = 0, y, i, b, r, v, found;
    found = 0;
    for (y = 17; !found && y < h && y < w; y += 2) {
        ave = 0;
        for (x = 0; x <= y; x++)
            ave += f[y - x][x];
        ave += y / 2;
        ave /= y + 1;
        b = 0, r = 0, i = 0;
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
        }
        if (i < 6)
            continue;
#if 0
        i = despeckle(i);
        if (i < 6)
            continue;
#endif
        r = 0;
        for (b = 1; b < i - 4; b += 2) {
            r += runs[b] + runs[b - 1];
            if (!isfinder(b))
                continue;
            x = runs[b + 2] / 2 + runs[b + 1] + r;
            y -= x;
            found = 1;
        }
    }
    if (!found)
        return 0;

    fx[0] = x;
    fy[0] = y;
    modwid = center(0);

    //    f[fy[0]][fx[0]] = 255;
    fprintf(stderr, "%d %d %d\n", x, y, i);
    return 1;
}

int findnexty( unsigned char t)
{
    int x = 0, y, i, b, r, v, found;
    avey = ave * 64;
    found = 0;
    x = fx[t];
    b = 0, r = 0, i = 0;
    runs[i] = 0;
    fprintf(stderr, "x=%d y=%d,", x, fy[t]);
    for (y = fy[t]; !found && y < h; y++) {
        avey += f[y][x];
        avey -= avey / 64;
        v = f[y][x] <= avey / 64;
        if (v == b) {
            r++;
            continue;
        }
        b = v;
        fprintf(stderr, "%d,", r);
        runs[i++] = r;
        runs[i] = 0;
        r = 1;
    }
    fprintf(stderr, " = %d\n", modwid);
    if (i < 6)
        return 0;
    r = 0;
    for (b = 1; b < i - 4; b += 2) {
        r += runs[b] + runs[b - 1];
        if (runs[b] * 8 < modwid || runs[b] * 2 > modwid)
            continue;
        if (!isfinder(b))
            continue;
        x = fy[t] + runs[b + 2] / 2 + runs[b + 1] + r;
        fprintf(stderr, "Y: %d\n", x);
        fy[cands] = x;
        fx[cands] = fx[t];
        center(cands);

        if (fw[cands] * 3 < modwid || fw[cands] > modwid)
            continue;
        if (fh[cands] * 3 < modwid || fh[cands] > modwid)
            continue;
        //        f[fy[cands]][fx[cands]] = 255;
        cands++;
        found = 1;
    }
    return found;
}

int findnextx(unsigned char t)
{
    int x = 0, y, i, b, r, v, found;
    avex = ave * 64;
    found = 0;
    y = fy[t];
    b = 0, r = 0, i = 0;
    runs[i] = 0;

    fprintf(stderr, "y=%d x=%d,", y, fx[t]);

    for (x = fx[t]; !found && x < w; x++) {
        avex += f[y][x];
        avex -= avex / 64;

        v = f[y][x] <= avex / 64;
        if (v == b) {
            r++;
            continue;
        }
        b = v;
        fprintf(stderr, "%d,", r);
        runs[i++] = r;
        runs[i] = 0;
        r = 1;
    }
    fprintf(stderr, "\n");
    if (i < 6)
        return 0;
    r = 0;
    for (b = 1; b < i - 4; b += 2) {
        r += runs[b] + runs[b - 1];
        if (runs[b] * 8 < modwid || runs[b] * 2 > modwid)
            continue;
        if (!isfinder(b))
            continue;
        x = fx[t] + runs[b + 2] / 2 + runs[b + 1] + r;
        fprintf(stderr, "X: %d\n", x);
        fx[cands] = x;
        fy[cands] = fx[t];
        center(cands);

        if (fw[cands] * 3 < modwid || fw[cands] > modwid)
            continue;
        if (fh[cands] * 3 < modwid || fh[cands] > modwid)
            continue;

        //        f[fy[cands]][fx[cands]] = 255;
        cands++;
        found = 1;
    }
    return found;
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
    while( i <  j) {
        findnexty(i);
        findnextx(i);
        i++;
    }
    i = j + 1;
    j = cands;
    while( i <  j) {
        findnexty(i);
        findnextx(i);
        i++;
    }

    unsigned x, y;
    for( x = 0 ; x < cands ; x++ ) {
        fprintf( stderr, "%d,%d %d %d\n", fx[x],fy[x],fw[x],fh[x] );
        f[fy[x]][fx[x]] = 255;
    }
    fprintf( stderr, "\n" );

    printf("P2\n%u %u\n255\n", w, h);
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
            printf("%u ", f[y][x]);

}
