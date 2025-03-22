/*----------------------------------------------------------------------*/
/*      LHarc Encoding/Decoding module              */
/*      This is part of LHarc UNIX Archiver Driver      */
/*                                  */
/*  LZSS Algorithm          Haruhiko.Okumura        */
/*  Adaptic Huffman Encoding    1989.05.27  Haruyasu.Yoshizaki  */
/*                                  */
/*                                  */
/*  V0.01  Modified for UNIX LHarc V0.01    1989.05.28  Y.Tagawa    */
/*  V0.02  Modified             1989.05.29  Y.Tagawa    */
/*  V0.03  Modified             1989.07.02  Y.Tagawa    */
/*  V0.03b Modified             1989.07.13  Y.Tagawa    */
/*  V0.03d Modified             1989.09.14  Y.Tagawa    */
/*  V1.00  Fixed                1989.09.22  Y.Tagawa    */
/*----------------------------------------------------------------------*/

/**************************************************************
** modified for use in DPBOX by Mark Wahl, DL4YBG            **
** modified for use in DPBOX by Joachim Schurig, DL8HBS      **
** last update 99/01/15                                      **
**************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "filesys.h"
#include "boxlocal.h"
#include "box_logs.h"
#include "shell.h"
#include "box_file.h"

#define maxcompfilesize 10000000 /* this is important */

static int in_memory, out_memory;
FILE *infile, *outfile;
static unsigned long int  textsize, codesize;
static char *srcbuf, *destbuf;
static char *srcbufptr, *destbufptr;
static long srclen, destlen;
static long srcbuflen, destbuflen;

/* read byte from input */
static int read_char()
{
  int chr;
  
  if (in_memory) {
    if (srclen < srcbuflen) {
      chr = *srcbufptr;
      srcbufptr++;
      srclen++;
      return(chr);
    }
    else {
      return(EOF);
    }
  }
  else {
    return(getc(infile));
  }
}

/* read byte from input, ignore EOF */
static unsigned int read_char1()
{
  unsigned int chr;
  
  if (in_memory) {
    if (srclen < srcbuflen) {
      chr = *srcbufptr;
      srcbufptr++;
      srclen++;
      return(chr);
    }
    else {
      return(0);
    }
  }
  else {
    if ((chr = getc(infile)) > 255) chr = 0;
    return(chr);
  }
}

/* write byte to output */
static int wri_char(chr)
int chr;
{
  if (out_memory) {
    if (destlen < destbuflen) {
      *destbufptr = (char) chr;
      destbufptr++;
      destlen++;
      return((char) chr);
    }
    else {
      return(EOF);
    }
  }
  else {
    return (putc(chr,outfile)); 
  }
}

/*----------------------------------------------------------------------*/
/*                                  */
/*      LZSS ENCODING                       */
/*                                  */
/*----------------------------------------------------------------------*/

#define N4K     4096    /* buffer size (original 4096) */
#define N2K     2048
#define F       60  /* pre-sence buffer size */
#define THRESHOLD   2
#define NIL     N2K /* term of tree */

static unsigned char    text_buf[N4K + F - 1];
static unsigned int match_position, match_length;
static int      lson[N4K + 1], rson[N4K + 1 + N4K], dad[N4K + 1];
static unsigned char    same[N4K + 1];


/* Initialize Tree */
static void InitTree () 
{
    register int *p, *e;

    for (p = rson + N2K + 1, e = rson + N4K + N4K; p <= e; )
        *p++ = NIL;
    for (p = dad, e = dad + N4K; p < e; )
        *p++ = NIL;
}

/* Insert to node */
static void InsertNode (r)
    register int r;
{
    register int        p;
    int         cmp;
    register unsigned char  *key;
    register unsigned int   c;
    register unsigned int   i, j;

    cmp = 1;
    key = &text_buf[r];
    i = key[1] ^ key[2];
    i ^= i >> 4;
    p = N2K + 1 + key[0] + ((i & 0x0f) << 8);
    rson[r] = lson[r] = NIL;
    match_length = 0;
    i = j = 1;
    for ( ; ; ) {
        if (cmp >= 0) {
            if (rson[p] != NIL) {
                p = rson[p];
                j = same[p];
            } else {
                rson[p] = r;
                dad[r] = p;
                same[r] = i;
                return;
            }
        } else {
            if (lson[p] != NIL) {
                p = lson[p];
                j = same[p];
            } else {
                lson[p] = r;
                dad[r] = p;
                same[r] = i;
                return;
            }
        }

        if (i > j) {
            i = j;
            cmp = key[i] - text_buf[p + i];
        } else
        if (i == j) {

            for (; i < F; i++)
                if ((cmp = key[i] - text_buf[p + i]) != 0)
                    break;
        }

        if (i > THRESHOLD) {
            if (i > match_length) {
                match_position = ((r - p) & (N2K - 1)) - 1;
                if ((match_length = i) >= F)
                    break;
            } else
            if (i == match_length) {
                if ((c = ((r - p) & (N2K - 1)) - 1) < match_position) {
                    match_position = c;
                }
            }
        }
    }
    same[r] = same[p];
    dad[r] = dad[p];
    lson[r] = lson[p];
    rson[r] = rson[p];
    dad[lson[p]] = r;
    dad[rson[p]] = r;
    if (rson[dad[p]] == p)
        rson[dad[p]] = r;
    else
        lson[dad[p]] = r;
    dad[p] = NIL;  /* remove p */
}

static void llink (n, p, q)
    int n, p, q;
{
    register unsigned char *s1, *s2, *s3;
    if (p >= NIL) {
        same[q] = 1;
        return;
    }
    s1 = text_buf + p + n;
    s2 = text_buf + q + n;
    s3 = text_buf + p + F;
    while (s1 < s3) {
        if (*s1++ != *s2++) {
            same[q] = s1 - 1 - text_buf - p;
            return;
        }
    }
    same[q] = F;
}


static void linknode (p, q, r)
    int p, q, r;
{
    int cmp;

    if ((cmp = same[q] - same[r]) == 0) {
        llink(same[q], p, r);
    } else if (cmp < 0) {
        same[r] = same[q];
    }
}

static void DeleteNode (p)
    register int p;
{
    register int  q;

    if (dad[p] == NIL)
        return;         /* has no linked */
    if (rson[p] == NIL) {
        if ((q = lson[p]) != NIL)
            linknode(dad[p], p, q);
    } else
    if (lson[p] == NIL) {
        q = rson[p];
        linknode(dad[p], p, q);
    } else {
        q = lson[p];
        if (rson[q] != NIL) {
            do {
                q = rson[q];
            } while (rson[q] != NIL);
            if (lson[q] != NIL)
                linknode(dad[q], q, lson[q]);
            llink(1, q, lson[p]);
            rson[dad[q]] = lson[q];
            dad[lson[q]] = dad[q];
            lson[q] = lson[p];
            dad[lson[p]] = q;
        }
        llink(1, dad[p], q);
        llink(1, q, rson[p]);
        rson[q] = rson[p];
        dad[rson[p]] = q;
    }
    dad[q] = dad[p];
    if (rson[dad[p]] == p)
        rson[dad[p]] = q;
    else
        lson[dad[p]] = q;
    dad[p] = NIL;
}

/*----------------------------------------------------------------------*/
/*                                  */
/*      HUFFMAN ENCODING                    */
/*                                  */
/*----------------------------------------------------------------------*/

#define N_CHAR      (256 - THRESHOLD + F) /* {code : 0 .. N_CHAR-1} */
#define T       (N_CHAR * 2 - 1)    /* size of table */
#define R       (T - 1)         /* root position */
#define MAX_FREQ    0x8000  /* tree update timing from frequency */

/* typedef unsigned char char; */



/* TABLE OF ENCODE/DECODE for upper 6bits position information */

/* for encode */
static char p_len[64] = {
    0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

static char p_code[64] = {
    0x00, 0x20, 0x30, 0x40, 0x50, 0x58, 0x60, 0x68,
    0x70, 0x78, 0x80, 0x88, 0x90, 0x94, 0x98, 0x9C,
    0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC,
    0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE,
    0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
    0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

/* for decode */
static char d_code[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
    0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
    0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
    0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
    0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
    0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
    0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
    0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
    0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
    0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
    0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

static char d_len[256] = {
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

static unsigned freq[T + 1];    /* frequency table */

static int prnt[T + N_CHAR];    /* points to parent node */
/* notes :
   prnt[T .. T + N_CHAR - 1] used by
   indicates leaf position that corresponding to code */

static int son[T];      /* points to son node (son[i],son[i+]) */

static unsigned getbuf = 0;
static char getlen = 0;


/* get one bit */
/* returning in Bit 0 */
static int GetBit ()
{
    register unsigned int dx = getbuf;
    register unsigned int c;

    if (getlen <= 8)
        {
            c = read_char1();
            dx |= c << (8 - getlen);
            getlen += 8;
        }
    getbuf = dx << 1;
    getlen--;
    return (dx & 0x8000) ? 1 : 0;
}

/* get one byte */
/* returning in Bit7...0 */
static int GetByte ()
{
    register unsigned int dx = getbuf;
    register unsigned c;

    if (getlen <= 8) {
        c = read_char1();
        dx |= c << (8 - getlen);
        getlen += 8;
    }
    getbuf = dx << 8;
    getlen -= 8;
    return (dx >> 8) & 0xff;
}

/* get N bit */
/* returning in Bit(N-1)...Bit 0 */
static int GetNBits (n)
    register unsigned int n;
{
    register unsigned int dx = getbuf;
    register unsigned int c;
    static int mask[17] = {
        0x0000,
        0x0001, 0x0003, 0x0007, 0x000f,
        0x001f, 0x003f, 0x007f, 0x00ff,
        0x01ff, 0x03ff, 0x07ff, 0x0fff,
        0x1fff, 0x3fff, 0x7fff, 0xffff };
    static int shift[17] = {
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    
    if (getlen <= 8)
        {
            c = read_char1();
            dx |= c << (8 - getlen);
            getlen += 8;
        }
    getbuf = dx << n;
    getlen -= n;
    return (dx >> shift[n]) & mask[n];
}

static unsigned putbuf = 0;
static char putlen = 0;

/* output C bits */
static int Putcode (l, c)
    register int l;
    register unsigned int c;
{
    register int len = putlen;
    register unsigned int b = putbuf;
    b |= c >> len;
    if ((len += l) >= 8) {
        if (wri_char(b >> 8) == EOF) return(1);
        if ((len -= 8) >= 8) {
            if (wri_char(b) == EOF) return(1);
            codesize += 2;
            len -= 8;
            b = c << (l - len);
        } else {
            b <<= 8;
            codesize++;
        }
    }
    putbuf = b;
    putlen = len;
    return(0);
}


/* Initialize tree */

static void StartHuff ()
{
    register int i, j;

    for (i = 0; i < N_CHAR; i++) {
        freq[i] = 1;
        son[i] = i + T;
        prnt[i + T] = i;
    }
    i = 0; j = N_CHAR;
    while (j <= R) {
        freq[j] = freq[i] + freq[i + 1];
        son[j] = i;
        prnt[i] = prnt[i + 1] = j;
        i += 2; j++;
    }
    freq[T] = 0xffff;
    prnt[R] = 0;
    putlen = getlen = 0;
    putbuf = getbuf = 0;
}


/* reconstruct tree */
static void reconst ()
{
    register int i, j, k;
    register unsigned f;

    /* correct leaf node into of first half,
       and set these freqency to (freq+1)/2       */
    j = 0;
    for (i = 0; i < T; i++) {
        if (son[i] >= T) {
            freq[j] = (freq[i] + 1) / 2;
            son[j] = son[i];
            j++;
        }
    }
    /* build tree.  Link sons first */
    for (i = 0, j = N_CHAR; j < T; i += 2, j++) {
        k = i + 1;
        f = freq[j] = freq[i] + freq[k];
        for (k = j - 1; f < freq[k]; k--);
        k++;
        {   register unsigned *p, *e;
            for (p = &freq[j], e = &freq[k]; p > e; p--)
                p[0] = p[-1];
            freq[k] = f;
        }
        {   register int *p, *e;
            for (p = &son[j], e = &son[k]; p > e; p--)
                p[0] = p[-1];
            son[k] = i;
        }
    }
    /* link parents */
    for (i = 0; i < T; i++) {
        if ((k = son[i]) >= T) {
            prnt[k] = i;
        } else {
            prnt[k] = prnt[k + 1] = i;
        }
    }
}


/* update given code's frequency, and update tree */

static void update (c)
    unsigned int    c;
{
    register unsigned *p;
    register int i, j, k, l;

    if (freq[R] == MAX_FREQ) {
        reconst();
    }
    c = prnt[c + T];
    do {
        k = ++freq[c];

        /* swap nodes when become wrong frequency order. */
        if (k > freq[l = c + 1]) {
            for (p = freq+l+1; k > *p++; ) ;
            l = p - freq - 2;
            freq[c] = p[-2];
            p[-2] = k;

            i = son[c];
            prnt[i] = l;
            if (i < T) prnt[i + 1] = l;

            j = son[l];
            son[l] = i;

            prnt[j] = c;
            if (j < T) prnt[j + 1] = c;
            son[c] = j;

            c = l;
        }
    } while ((c = prnt[c]) != 0);   /* loop until reach to root */
}

/* static unsigned code, len; */

static int Encodechar (c)
    unsigned c;
{
    register int *p;
    register unsigned long i;
    register int j, k;

    i = 0;
    j = 0;
    p = prnt;
    k = p[c + T];

    /* trace links from leaf node to root */
    do {
        i >>= 1;

        /* if node index is odd, trace larger of sons */
        if (k & 1) i += 0x80000000;

        j++;
    } while ((k = p[k]) != R) ;
    if (j > 16) {
        if (Putcode(16, (unsigned int)(i >> 16))) return(1);
        if (Putcode(j - 16, (unsigned int)i)) return(1);
    } else {
        if (Putcode(j, (unsigned int)(i >> 16))) return(1);
    }
/*  code = i; */
/*  len = j; */
    update(c);
    return(0);
}

static int EncodePosition (c)
    unsigned c;
{
    unsigned i;

    /* output upper 6bit from table */
    i = c >> 6;
    if (Putcode((int)(p_len[i]), (unsigned int)(p_code[i]) << 8)) return(1);

    /* output lower 6 bit */
    if (Putcode(6, (unsigned int)(c & 0x3f) << 10)) return(1);
    return(0);
}

static int EncodeEnd()
{
        if (putlen) {
                if (wri_char(putbuf >> 8) == EOF) return(1);
                codesize++;
        }
        return(0);
}


static int Decodechar ()
{
    register unsigned c;

    c = son[R];

    /* trace from root to leaf,
       got bit is 0 to small(son[]), 1 to large (son[]+1) son node */
    while (c < T) {
        c += GetBit();
        c = son[c];
    }
    c -= T;
    update(c);
    return c;
}

static int DecodePosition ()
{
    unsigned i, j, c;

    /* decode upper 6bit from table */
    i = GetByte();
    c = (unsigned)d_code[i] << 6;
    j = d_len[i];

    /* get lower 6bit */
    j -= 2;
    return c | (((i << j) | GetNBits (j)) & 0x3f);
}


/* compression */

static int Encode ()
{
    register int  i, c, len, r, s, last_match_length;

    textsize = 0;
    StartHuff();
    InitTree();
    s = 0;
    r = N4K - F;
    for (i = s; i < r; i++)
        text_buf[i] = ' ';
    r = N2K - F; /* this is important! */
    for (len = 0; len < F && (c = read_char()) != EOF; len++)
        text_buf[r + len] = c;
    textsize = len;
    for (i = 1; i <= F; i++)
        InsertNode(r - i);
    InsertNode(r);
    do {
        if (match_length > len)
            match_length = len;
        if (match_length <= THRESHOLD) {
            match_length = 1;
            if (Encodechar(text_buf[r])) return(1);
        } else {
            if (Encodechar(255 - THRESHOLD + match_length)) return(1);
            if (EncodePosition(match_position)) return(1);
        }
        last_match_length = match_length;
        for (i = 0; i < last_match_length && (c = read_char()) != EOF; i++) {
            DeleteNode(s);
            text_buf[s] = c;
            if (s < F - 1)
                text_buf[s + N2K] = c;
            s = (s + 1) & (N2K - 1);
            r = (r + 1) & (N2K - 1);
            InsertNode(r);
        }

        textsize += i;
        while (i++ < last_match_length) {
            DeleteNode(s);
            s = (s + 1) & (N2K - 1);
            r = (r + 1) & (N2K - 1);
            if (--len) InsertNode(r);
        }
    } while (len > 0);
    if (EncodeEnd()) return(1);
    return(0);
}

static int Decode(textsize)  /* recover */
unsigned long int textsize;
{
    register int    i, j, k, r, c;
    register unsigned long int count;

    StartHuff();
    r = N4K - F;
    for (i = 0; i < r; i++)
        text_buf[i] = ' ';
    for (count = 0; count < textsize; ) {
        c = Decodechar();
        if (c < 256) {
            if (wri_char(c) == EOF) return(1);
            text_buf[r++] = c;
            r &= (N4K - 1);
            count++;
        } else {
            i = (r - DecodePosition() - 1) & (N4K - 1);
            j = c - 255 + THRESHOLD;
            for (k = 0; k < j; k++) {
                c = text_buf[(i + k) & (N4K - 1)];
                if (wri_char(c) == EOF) return(1);
                text_buf[r++] = c;
                r &= (N4K - 1);
                count++;
            }
        }

    }
    return(0);
}


static void init_huf()
{
  textsize = 0;
  codesize = 0;
  getbuf = 0;
  getlen = 0;
  putbuf = 0;
  putlen = 0;
}

int enchuf(gzip,preserve_original,inputfile,outputfile,crlfconv)
char *inputfile;
char *outputfile;
int crlfconv, gzip, preserve_original;
{
  int error;
  char tempname[80], hs[256];
  char *inputfptr;
  int conv;
  int bin;
  int prebin;
  int bintest;
  int c;

  debug0(2, -1, 211);
  conv = 0;
  if (crlfconv) {
    bin = 0;
    prebin = 0;
    bintest = 0;
    strcpy(tempname, tempdir);
    strcat(tempname,"hufXXXXXX");
    mymktemp(tempname);
    if ((infile = fopen(inputfile,"rb")) == NULL) {
      return(1);
    }
    if ((outfile = fopen(tempname,"wb")) == NULL) {
      fclose(infile);
      return(1);
    }
    while ((c = getc(infile)) != EOF) {
      if (!bin) {
        if ((char)c == '\n') {
          putc('\r',outfile);
          if (prebin) bin = 1;
        }
      }
      putc(c,outfile);
      if (!prebin) {
        switch (bintest) {
        case 0:
          if ((char)c == '\n') bintest++;
          break;
        case 1:
          if ((char)c == '#') bintest++;
          else if ((char)c != '\n') bintest = 0;
          break;
        case 2:
          if ((char)c == 'B') bintest++;
          else bintest = 0;
          break;
        case 3:
          if ((char)c == 'I') bintest++;
          else bintest = 0;
          break;
        case 4:
          if ((char)c == 'N') bintest++;
          else bintest = 0;
          break;
        case 5:  
          if ((char)c == '#') bintest++;
          else bintest = 0;
          break;
        case 6:  
          if (isdigit(c)) prebin = 1;
          else bintest = 0;
          break;
        default:
          bintest = 0;
          break;
        }
      }
    }
    fclose(infile);
    fclose(outfile);
    inputfptr = tempname;
    conv = 1;
  }
  else {
    inputfptr = inputfile;
  }
  
  if (gzip) {
    if (preserve_original && !conv) {
      strcpy(tempname, tempdir);
      strcat(tempname,"hufXXXXXX");
      mymktemp(tempname);
      filecopy(inputfptr, tempname);
      inputfptr = tempname;
    }
    sprintf(hs, "gzip -fnq5 %s", inputfptr);
    my_system(hs);
    strcpy(hs, inputfptr);
    strcat(hs, ".gz");
    if (!exist(hs)) {
      if (conv || preserve_original) sfdelfile(tempname);
      return 1;
    }
    sfrename(hs, outputfile);
    return 0;
  }
  
  in_memory = 0;
  out_memory = 0;
  init_huf();
  if ((infile = fopen(inputfptr,"rb")) == NULL) {
    if (conv) {
      unlink(tempname);
    }
    return(1);
  }
  if ((outfile = fopen(outputfile,"wb")) == NULL) {
    fclose(infile);
    if (conv) {
      unlink(tempname);
    }
    return(1);
  } 
  error = 0;
  fseek(infile, 0L, 2);
  textsize = ftell(infile);
  if (fwrite(&textsize, sizeof textsize, 1, outfile) < 1) {
    error = 1;
  }
  if (!error) {
    if (textsize == 0)
      error = 1;
    if (!error) {
      rewind(infile);
      if (Encode())
        error = 1;
    }
  }
  fclose(infile);
  fclose(outfile);
  if (conv) {
    unlink(tempname);
  }
  return(error);
}

int dechuf(gzip,preserve_original,inputfile,outputfile,crlfconv)
char *inputfile;
char *outputfile;
int crlfconv, gzip, preserve_original;
{
  int error;
  char tempname[80], tempname2[80], hs[256], oname[256];
  char *outputfptr;
  int conv;
  int bin;
  int prebin;
  int bintest;
  int c;
  int only_cr;

  debug0(2, -1, 212);
  conv = 0;
  error = 0;

  outputfptr = outputfile;
  if (crlfconv) {
    strcpy(tempname, tempdir); 
    strcat(tempname,"hufXXXXXX");
    mymktemp(tempname);
    conv = 1;
    outputfptr = tempname;
  }    

  if (gzip) {
    if (preserve_original) {
      strcpy(tempname2, tempdir);
      strcat(tempname2,"gzipXXXXXX");
      mymktemp(tempname2);
      strcpy(oname, tempname2);
      strcat(tempname2, ".gz");
      filecopy(inputfile, tempname2);
    } else {
      strcpy(tempname2, inputfile);
      strcpy(oname, tempname2);
      strcat(tempname2, ".gz");
      sfrename(inputfile, tempname2);
    }
    sprintf(hs, "gzip -dfnq %s", tempname2);
    my_system(hs);
    if (!exist(oname)) {
      if (preserve_original) sfdelfile(tempname2);
      return 1;
    }
    sfrename(oname, outputfptr);
  }
  
  else {
  
    in_memory = 0;
    out_memory = 0;
    init_huf();
    if ((infile = fopen(inputfile,"rb")) == NULL) {
      return(1);
    }
    if ((outfile = fopen(outputfptr,"wb")) == NULL) {
      fclose(infile);
      return(1);
    } 
    if (fread(&textsize, sizeof textsize, 1, infile) < 1)
      error = 1;
    if (!error) {
      if (textsize == 0)
        error = 1;
      if (!error) {
        if (textsize > maxcompfilesize)
          error = 1;
        if (!error) {
          if (Decode(textsize))
            error = 1;
        }
      }
    }
    fclose(infile);
    fclose(outfile);
  }
  
  if ((conv) && (!error)) {
    bin = 0;
    prebin = 0;
    bintest = 0;
    if ((infile = fopen(tempname,"rb")) == NULL) {
      return(1);
    }
    if ((outfile = fopen(outputfile,"wb")) == NULL) {
      fclose(infile);
      unlink(tempname);
      return(1);
    }
    only_cr = 0;
    while ((c = getc(infile)) != EOF) {
      if (!bin) {
        if (only_cr && ((char)c != '\n')) putc('\n',outfile);
        only_cr = 0;
      }
      if (bin || ((char)c != '\r')) putc(c,outfile);
      if (!bin) {
        if ((char)c == '\r') only_cr = 1;
        if (prebin && ((char)c == '\n')) bin = 1;
      }
      if (!prebin) {
        switch (bintest) {
        case 0:
          if ((char)c == '\n') bintest++;
          break;
        case 1:
          if ((char)c == '#') bintest++;
          else if ((char)c != '\n') bintest = 0;
          break;
        case 2:
          if ((char)c == 'B') bintest++;
          else bintest = 0;
          break;
        case 3:
          if ((char)c == 'I') bintest++;
          else bintest = 0;
          break;
        case 4:
          if ((char)c == 'N') bintest++;
          else bintest = 0;
          break;
        case 5:  
          if ((char)c == '#') bintest++;
          else bintest = 0;
          break;
        case 6:  
          if (isdigit(c)) prebin = 1;
          else bintest = 0;
          break;
        default:
          bintest = 0;
          break;
        }
      }
    }
    fclose(infile);
    fclose(outfile);
  }
  if (conv) {
    unlink(tempname);
  }
  return(error);
}

int enchufmem(gzip,membase,osize,outbase,outsize,outputfile,crlfconv)
char *membase;
long osize;
char **outbase;
long *outsize;
char *outputfile;
int crlfconv, gzip;
{
  int error;
  char *ptr;
  int i;
  long size;
  int conv;
  char *tmpbuf = NULL;
  long tmplen;
  int bin;
  int prebin;
  int bintest;
  char c;
  char *tmpptr;
  char *srcptr;
  char tempname[80];

  debug0(2, -1, 213);
  if (osize == 0) return(1);
  srcbuf = membase;
  size = osize;
  conv = 0;

  if (crlfconv) {
    tmpbuf = malloc(osize*2);
    if (tmpbuf == NULL) return(1);
    tmpptr = tmpbuf;
    tmplen = 0;
    srcbuf = membase;
    srcptr = srcbuf;
    srclen = 0;
    bin = 0;
    prebin = 0;
    bintest = 0;
    
    while (srclen < osize) {
      if (!bin) {
        if (*(srcptr) == '\n') {
          *(tmpptr) = '\r';
          tmpptr++;
          tmplen++;
          if (prebin) bin = 1;
        }
      }
      *(tmpptr) = *(srcptr);
      if (!prebin) {
        c = *(srcptr);
        switch (bintest) {
        case 0:
          if (c == '\n') bintest++;
          break;
        case 1:
          if (c == '#') bintest++;
          else if (c != '\n') bintest = 0;
          break;
        case 2:
          if (c == 'B') bintest++;
          else bintest = 0;
          break;
        case 3:
          if (c == 'I') bintest++;
          else bintest = 0;
          break;
        case 4:
          if (c == 'N') bintest++;
          else bintest = 0;
          break;
        case 5:  
          if (c == '#') bintest++;
          else bintest = 0;
          break;
        case 6:  
          if (isdigit((int)c)) prebin = 1;
          else bintest = 0;
          break;
        default:
          bintest = 0;
          break;
        }
      }
      tmplen++;
      srclen++;
      tmpptr++;
      srcptr++;
    }
    size = tmplen;
    srcbuf = tmpbuf;
    conv = 1;
  }

  if (gzip) {
    strcpy(tempname, tempdir);
    strcat(tempname,"gzipXXXXXX");
    mymktemp(tempname);
    if ((outfile = fopen(tempname,"wb")) == NULL) {
      if (conv)
        free(tmpbuf);
      return(1);
    } 
    if (fwrite(srcbuf, size, 1, outfile) != 1) {
      fclose(outfile);
      sfdelfile(tempname);
      if (conv)
        free(tmpbuf);
      return 1;
    }
    fclose(outfile);
    if (conv)
      free(tmpbuf);

    error = enchuf(1,0,tempname,outputfile,0);
    sfdelfile(tempname);
    *outbase = NULL;
    *outsize = 0;
    return error;
  }

  in_memory = 1;
  out_memory = 1;
  init_huf();

  srcbufptr = srcbuf;
  srclen = 0;
  srcbuflen = size;

  /* first try memory */
  error = 0;
  destlen = 0;
  destbuflen = size + size/6;
  destbuf = malloc(destbuflen);   /* allocate buffer with security margin */
  if (destbuf != NULL) {
    destbufptr = destbuf;
    ptr = (char *)&size;
    /* send length */
    for (i=0;i<sizeof(size);i++) {
      if (wri_char(*ptr++) == EOF) {
        error = 1;
        break;
      }
    }
    if (!error) {
      if (!Encode()) {
        *outbase = destbuf;
        *outsize = destlen;
        if (conv)
          free(tmpbuf);
        return(0);
      }
    }
  }
  free(destbuf);

  /* output to memory failed, write to file */
  out_memory = 0;
  init_huf();

  srcbufptr = srcbuf;
  srclen = 0;
  *outbase = NULL;
  *outsize = 0;

  if ((outfile = fopen(outputfile,"wb")) == NULL) {
    if (conv)
      free(tmpbuf);
    return(1);
  } 
  error = 0;
  if (fwrite(&size, sizeof size, 1, outfile) < 1) {
    error = 1;
  }
  if (!error) {
    if (Encode())
      error = 1;
  }
  fclose(outfile);
  if (conv)
    free(tmpbuf);
  return(error);
}

int dechufmem(gzip,membase,size,outbase,outsize,outputfile,crlfconv)
char *membase;
long size;
char **outbase;
long *outsize;
char *outputfile;
int crlfconv, gzip;
{
  int error;
  char *ptr;
  int i;
  long packsize;
  char tempname[80];
  char *outputfptr;
  int conv;
  long worlen;
  long tmplen;
  char *tmpbuf;
  int bin;
  int prebin;
  int bintest;
  int c;
  int only_cr;

  debug0(2, -1, 214);
  if (gzip) {
    strcpy(tempname, tempdir);
    strcat(tempname,"gzipXXXXXX");
    mymktemp(tempname);
    strcat(tempname,".gz");
    if ((outfile = fopen(tempname,"wb")) == NULL) {
      return(1);
    } 
    error = 0;
    if (fwrite(membase, size, 1, outfile) != 1) {
      fclose(outfile);
      sfdelfile(tempname);
      return 1;
    }
    fclose(outfile);

    error = dechuf(1,0,tempname,outputfile,crlfconv);
    sfdelfile(tempname);
    *outbase = NULL;
    *outsize = 0;
    return error;
  }

  conv = 0;
  outputfptr = outputfile;
  if (crlfconv) {
    strcpy(tempname, tempdir);
    strcat(tempname,"hufXXXXXX");
    mymktemp(tempname);
    conv = 1;
    outputfptr = tempname;
  }

  in_memory = 1;
  out_memory = 1;
  init_huf();

  srcbuf = membase;
  srcbufptr = srcbuf;
  srclen = 0;
  srcbuflen = size;
  if (srcbuflen < sizeof(packsize)) return(1);
  /* get length */
  ptr = (char *)&packsize;
  for (i=0;i<sizeof(packsize);i++) {
    *ptr++ = read_char();
  }
  if (packsize == 0) return(1);

  /* first try memory */
  error = 0;
  destlen = 0;
  destbuflen = packsize;
  destbuf = malloc(destbuflen);   /* allocate buffer */
  if (destbuf != NULL) {
    destbufptr = destbuf;
    if (!Decode(packsize)) {
      if (conv) {
        tmpbuf = malloc(packsize);
        if (tmpbuf != NULL) {
          tmplen = 0;
          worlen = 0;
          bin = 0;
          prebin = 0;
          bintest = 0;
          only_cr = 0;
          while (worlen < packsize) {
            if (!bin) {
              if (only_cr && (*(destbuf + worlen) != '\n')) {
                *(tmpbuf + tmplen) = '\n';
                tmplen++;
              }
              only_cr = 0;
            }
            if (bin || (*(destbuf + worlen) != '\r')) {
              *(tmpbuf + tmplen) = *(destbuf + worlen);
              tmplen++;
            }
            if (!bin && (*(destbuf + worlen) == '\r')) only_cr = 1;
            if (prebin && (*(destbuf + worlen) == '\n')) bin = 1;
            if (!prebin) {
              c = (int)(*(destbuf + worlen));
              switch (bintest) {
              case 0:
                if ((char)c == '\n') bintest++;
                break;
              case 1:
                if ((char)c == '#') bintest++;
                else if ((char)c != '\n') bintest = 0;
                break;
              case 2:
                if ((char)c == 'B') bintest++;
                else bintest = 0;
                break;
              case 3:
                if ((char)c == 'I') bintest++;
                else bintest = 0;
                break;
              case 4:
                if ((char)c == 'N') bintest++;
                else bintest = 0;
                break;
              case 5:  
                if ((char)c == '#') bintest++;
                else bintest = 0;
                break;
              case 6:  
                if (isdigit(c)) prebin = 1;
                else bintest = 0;
                break;
              default:
                bintest = 0;
                break;
              }
            }
            worlen++;
          }
          *outbase = tmpbuf;
          *outsize = tmplen;
          free(destbuf);
          return(0);
        }
      }
      else {
        *outbase = destbuf;
        *outsize = packsize;
        return(0);
      }
    }
  }
  free(destbuf);

  /* output to memory failed, write to file */
  out_memory = 0;
  init_huf();

  srcbuf = membase;
  srcbufptr = srcbuf + sizeof(packsize);
  srclen = sizeof(packsize);
  srcbuflen = size;
  *outbase = NULL;
  *outsize = 0;

  if ((outfile = fopen(outputfptr,"wb")) == NULL) {
    return(1);
  } 
  if (Decode(packsize))
    error = 1;
  fclose(outfile);
  if ((conv) && (!error)) {
    bin = 0;
    bintest = 0;
    prebin = 0;
    if ((infile = fopen(tempname,"rb")) == NULL) {
      return(1);
    }
    if ((outfile = fopen(outputfile,"wb")) == NULL) {
      fclose(infile);
      unlink(tempname);
      return(1);
    }
    only_cr = 0;
    while ((c = getc(infile)) != EOF) {
      if (!bin) {
        if (only_cr && ((char)c != '\n')) {
          putc('\n',outfile);
        }
        only_cr = 0;
      }
      if (bin || ((char)c != '\r')) putc(c,outfile);
      if (!bin) {
        if ((char)c == '\r') only_cr = 1;
        if (prebin && ((char)c == '\n')) bin = 1;
      }
      if (!prebin) {
        switch (bintest) {
        case 0:
          if ((char)c == '\n') bintest++;
          break;
        case 1:
          if ((char)c == '#') bintest++;
          else if ((char)c != '\n') bintest = 0;
          break;
        case 2:
          if ((char)c == 'B') bintest++;
          else bintest = 0;
          break;
        case 3:
          if ((char)c == 'I') bintest++;
          else bintest = 0;
          break;
        case 4:
          if ((char)c == 'N') bintest++;
          else bintest = 0;
          break;
        case 5:  
          if ((char)c == '#') bintest++;
          else bintest = 0;
          break;
        case 6:  
          if (isdigit(c)) prebin = 1;
          else bintest = 0;
          break;
        default:
          bintest = 0;
          break;
        }
      }
    }
    fclose(infile);
    fclose(outfile);
  }
  if (conv) {
    unlink(tempname);
  }
  return(error);
}

