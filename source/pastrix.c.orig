/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/

#ifndef HAS_MEMCHR
#define HAS_MEMCHR
#endif

#define PASTRIX_G
#include <string.h>
#include "pastrix.h"
#include "boxglobl.h"
#include "boxlocal.h"
#include "tools.h"

#if defined(__linux__) || defined(__NetBSD__)
#include <ctype.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#else
#include <time.h>
#endif
#ifdef __macos__
#include <timer.h>
#endif

#ifdef __macos__
long memavail__(void)
{
  long total, cont;
  
  PurgeSpace(&total, &cont);
  return(total);
}

long maxavail__(void)
{
  long total, cont;
  
  PurgeSpace(&total, &cont);
  return(cont);
}  
#endif



/* Fragt einen 200Hz-Systemzaehler ab. Der absolute Betrag ist unerheblich,  */
/* DP bildet immer die Differenz aus zwei Werten. Wird fuer die Box ge-      */
/* braucht, um die CPU-Time des Benutzers zu ermitteln.                      */
/* Man kann also beliebige Zaehler verwenden, einen 50Hz-Zaehler sollte man  */
/* aber jeweils mit 4 multiplizieren, da sonst alle Timeoutwerte und alle    */
/* statistischen Angaben ueber die Boxauslastung falsch sind                 */
/* Mittlerweile ist das oben geschriebene obsolet. Abgefragt wird ein 1MHZ-  */
/* Zähler, das Ergebnis wird in TICKSPERSEC Hz angegeben.		     */

long statclock(void)
{
#ifdef __macos__
  struct UnsignedWide microTickCount;
  
  Microseconds(&microTickCount);
  return((microTickCount.hi / ((1000000 / TICKSPERSEC)-1))
	+ (microTickCount.lo / (1000000 / TICKSPERSEC)));
  /* muss noch korrigiert werden, reicht aber erstmal zum testen */
  
#else
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv,&tz);
  return(((tv.tv_sec % ((LONG_MAX / TICKSPERSEC)-1)) * TICKSPERSEC)
	+ (tv.tv_usec / (1000000 / TICKSPERSEC)));
#endif
}

#if defined(__linux__) || defined(__NetBSD__)

long get_cpuusage(void)
{
  struct rusage usage;

  if (!getrusage(RUSAGE_SELF, &usage))
    return (((usage.ru_utime.tv_sec % ((LONG_MAX / TICKSPERSEC)-1)) * TICKSPERSEC)
	   + (usage.ru_utime.tv_usec / (1000000 / TICKSPERSEC))
    	  + ((usage.ru_stime.tv_sec % ((LONG_MAX / TICKSPERSEC)-1)) * TICKSPERSEC)
	   + (usage.ru_stime.tv_usec / (1000000 / TICKSPERSEC)));
  else
    return statclock();
}


long get_memusage(void)
{
  struct rusage usage;

  if (!getrusage(RUSAGE_SELF, &usage))
    return (usage.ru_ixrss + usage.ru_idrss + usage.ru_isrss);
  else
    return 0;
}

#endif


void mtpause(void)
{
  /* hier koennte sich der prozess ein wenig schlafenlegen	*/
  /* wird aber im Source nicht verwendet			*/
  /* usleep(100);						*/
}


long searchbyte(char what, register char *p, long size)
{
#ifdef HAS_MEMCHR

  register char *p2;

  p2 = memchr(p, what, size);
  if (p2 == NULL) return 0;
  else return (p2-p);

#else

  register long z = 0;

  if (p == NULL)
    return 0;

  while (z++ < size && *p++ != what);

  if (z >= size)
    return 0;
  return --z;

#endif
}


void ersetze(char *oldstr, char *newstr, char *txt)
{
  short k, s, d;

  k = strpos2(txt, oldstr, 1);
  if (k <= 0)
    return;
  s = strlen(oldstr);
  d = strlen(newstr) - s;
  while (k > 0 && strlen(txt) + d <= 255) {
    strdelete(txt, k, s);
    strinsert(newstr, txt, k);
    k = strpos2(txt, oldstr, 1);
  }
}


typedef char es[2];
typedef char ess[3];
typedef char esss[4];

static es e1 = "[", e2 = "]", e3 = "\\", e4 = "{", e5 = "|", e6 = "}",
	  e7 = "~", e10 = "\204", e11 = "\224", e12 = "\201", e13 = "\216",
	  e14 = "\231", e15 = "\232", e16 = "\341", e17 = "\236";
static ess es1 = "ae", es2 = "oe", es3 = "ue", es4 = "Ae", es5 = "Oe",
	   es6 = "Ue", es7 = "ss";
static esss ess1 = "sss";

void umlaut1(char *txt)
{
  ersetze(e10, es1, txt);
  ersetze(e11, es2, txt);
  ersetze(e12, es3, txt);
  ersetze(e13, es4, txt);
  ersetze(e14, es5, txt);
  ersetze(e15, es6, txt);
  ersetze(e16, es7, txt);
  ersetze(e17, es7, txt);
  ersetze(ess1, es7, txt);
}


void umlaut2(char *txt)
{
  ersetze(e1, e13, txt);
  ersetze(e2, e14, txt);
  ersetze(e3, e15, txt);
  ersetze(e4, e10, txt);
  ersetze(e5, e11, txt);
  ersetze(e6, e12, txt);
  ersetze(e7, e17, txt);
}

#ifdef __macos__

int fork()
{
    return -1;
}

int kill(pid_t pid, int signal)
{
    return 0;
}

void setsid()
{
}

int waitpid(pid_t pid, int *res, int flag)
{
    return -1;
}

#endif

/* Wandelt einen String mit Namen in richtige Gross/Kleinschreibung */

void gkdeutsch(char *name)
{
  short x, k;

  k = strlen(name);
  if (k == 0)
    return;
  lower(name);
  name[0] = upcase_(name[0]);
  for (x = 1; x < k; x++) {
    switch (name[x - 1]) {

    case ' ':
    case '-':
    case '_':
    case '(':
    case '[':
    case '.':
    case '/':
    case ':':
    case '|':
    case ',':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      name[x] = upcase_(name[x]);
      break;

    }
  }
}


/* Liest ein File in PUFFER und beachtet dabei die maximal moeglichen */
/* Memory Blocks                                                      */

void sfbread(boolean aslongaspossible, char *name, char **puffer, long *size)
{
  long fs, err;
  long hsz;
  char *fp;
  short k;

  fp = NULL;
  *puffer = NULL;
  *size = 0;

  k = sfopen(name, FO_READ);
  if (k < minhandle)
    return;

  hsz = sfseek(0, k, SFSEEKEND);
  sfseek(0, k, SFSEEKSET);

  if (hsz > maxram()) {
    if (aslongaspossible)
      fs = maxram();
    else
      fs = 0;
  } else
    fs = hsz;
    
  if (fs > 0)
    fp = malloc(fs);

  if (fs > 0 && fp == NULL) {
    if (aslongaspossible) {
      fs = maxavail__();
      fp = malloc(fs);
    }
  }

  if (fp != NULL) {
    err = sfread(k, fs, fp);
    if (err == fs) {
      *puffer = fp;
      *size = fs;
    } else
      free(fp);
  }

  sfclose(&k);
}

/* the memmem function is broken in most linux libs. Here´s my own version  */
/* (limitation: needle may not contain zero bytes)    	      	      	    */

char *mymemmem(register char *haystack, register long haystacksize,
      	      	register char *needle, register long needlesize)
{
  register char	*ret;

  if (needlesize <= 0 || haystacksize < needlesize) return (NULL);
  while ((ret = memchr(haystack, *needle, haystacksize)) != NULL) {
    haystacksize -= ((long)ret - (long)haystack);
    if (haystacksize < needlesize) return (NULL);
    haystacksize--;
    haystack = ret;
    haystack++;
    if (!strncmp(ret, needle, needlesize)) return (ret);
  }
  return (NULL);
}


/* Common string functions, taken from p2c.c of Dave Gillespie: */

/* Store in "ret" the substring of length "len" starting from "pos" (1-based).
   Store a shorter or null string if out-of-range.  Return "ret". */

char *strsub(ret, s, pos, len)
register char *ret, *s;
register int pos, len;
{
    register char *s2;

    if (--pos < 0 || len <= 0) {
        *ret = 0;
        return ret;
    }
    while (pos > 0) {
        if (!*s++) {
            *ret = 0;
            return ret;
        }
        pos--;
    }
    s2 = ret;
    while (--len >= 0) {
        if (!(*s2++ = *s++))
            return ret;
    }
    *s2 = 0;
    return ret;
}


/* Return the index of the first occurrence of "pat" as a substring of "s",
   starting at index "pos" (1-based).  Result is 1-based, 0 if not found. */

int strpos2(s, pat, pos)
char *s;
register char *pat;
register int pos;
{
    register char *cp, ch;
    register int slen;

    if (--pos < 0)
        return 0;
    slen = strlen(s) - pos;
    cp = s + pos;
    if (!(ch = *pat++))
        return 0;
    pos = strlen(pat);
    slen -= pos;
    while (--slen >= 0) {
        if (*cp++ == ch && !strncmp(cp, pat, pos))
            return cp - s;
    }
    return 0;
}


/* Delete the substring of length "len" at index "pos" from "s".
   Delete less if out-of-range. */
void strdelete(s, pos, len)
register char *s;
register int pos, len;
{
    register int slen;

    if (--pos < 0)
        return;
    slen = strlen(s) - pos;
    if (slen <= 0)
        return;
    s += pos;
    if (slen <= len) {
        *s = 0;
        return;
    }
    while ((*s = s[len])) s++;
}


/* Insert string "src" at index "pos" of "dst". */

void strinsert(src, dst, pos)
register char *src, *dst;
register int pos;
{
    register int slen, dlen;

    if (--pos < 0)
        return;
    dlen = strlen(dst);
    dst += dlen;
    dlen -= pos;
    if (dlen <= 0) {
        strcpy(dst, src);
        return;
    }
    slen = strlen(src);
    do {
        dst[slen] = *dst;
        --dst;
    } while (--dlen >= 0);
    dst++;
    while (--slen >= 0)
        *dst++ = *src++;
}


/* end of p2c-subroutines */



static void del_lead0(register char *s)
{
  /*Loescht fuehrende '00' in "s"*/
  register char *p;

  p = s;
  while (*s == '0') s++;
  if (s == p) return;
  if (*s == '\0') {
    /* eine Null stehen lassen */
    p[1] = '\0';
    return;
  }
  while ((*p++ = *s++));
}

void del_allblanks(register char *s)
{
  /* loescht alle Leerzeichen in s */
  register char *p;
  register char c;

  p = s;
  while ((c = *p++)) {
    if (c != ' ' && c != tab) *s++ = c;
  }
  *s = '\0';
}

void del_leadblanks(register char *s)
{
  /*Loescht fuehrende Leerzeichen in "s"*/
  register char *p;

  p = s;
  while (*s == ' ' || *s == tab) s++;
  if (s == p) return;
  while ((*p++ = *s++));
}


void del_lastblanks(register char *s)
{
  /*Loescht letzte Leerzeichen*/
  register char *p;
  
  p = s;
  if (*s++ == '\0') return;
  while (*s++ != '\0');
  s--;
  s--;
  while (s >= p && (*s == ' ' || *s == tab)) s--;
  s++;
  *s = '\0';
}


/*Loescht Spaces an Anfang und Ende des Strings*/

void del_blanks(char *s)
{
  del_lastblanks(s);
  del_leadblanks(s);
}


void lspacing(register char *txt, register short l)
{
  register short i = 0;
  register char *p;
  register char *s;
  
  p = txt;
  while (*txt++ != '\0') i++;
  i = l - i;  /* soviele Zeichen fehlen noch   */
  if (i <= 0) return;
  txt--;
  s = txt;
  txt = txt + i;
  while (s >= p) *txt-- = *s--;
  while (txt >= p) *txt-- = ' ';
}


void rspacing(register char *txt, short l)
{
  register char *e;
  
  e = txt + l;
  while (*txt++ != '\0');
  txt--;
  while (txt < e) *txt++ = ' ';
  *txt = '\0';
}


char lowcase(register char ch)
{
  switch (ch) {

  case 142:
    return 132;

  case 153:
    return 148;

  case 154:
    return 129;

  default:
    return tolower(ch);
  }
}


char upcase_(register char ch)
{
  switch (ch) {

  case 132:
    return 142;

  case 148:
    return 153;

  case 129:
    return 154;
 
  default:
    return toupper(ch);
  }
}


void upper(char *s)
{
  while ((*s++ = upcase_(*s)));
}

void strcpyupper(char *outs, char *ins)
{
  while ((*outs++ = upcase_(*ins++)));
}

void lower(char *s)
{
  while ((*s++ = lowcase(*s)));
}

void strcpylower(char *outs, char *ins)
{
  while ((*outs++ = lowcase(*ins++)));
}


/* --------------------------------------------------------------------- */


boolean zahl(register char *s)
{
  /* nur dez               */

  if (*s == '\0') return false;
  if (*s == '-' || *s == '+') {
    s++;
    if (*s == '\0') return false;
  }
  while (*s != '\0') {
    if (!isdigit(*s++)) {
      return false;
    }
  }
  return true;
}

boolean azahl(char *s)
{
  /* auch bin und hex      */

  if (*s == '\0')
    return false;

  if (*s == '$') {
    if (s[1] == '\0')
      return false;
    while (*s != '\0') {
      if (!((*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F') || isdigit(*s)))
	return false;
      s++;
    }
    return true;
  }

  if (*s != '%') {
    return zahl(s);
  }

  if (s[1] == '\0')
    return false;

  while (*s != '\0') {
    if (*s < '0' || *s > '1')
      return false;
    s++;
  }
  return true;
}


boolean rzahl(char *s)
{
  boolean digit;

  if (*s == '\0')
    return false;
  if (s[1] == '\0')
    return (zahl(s) && *s != '+' && *s != '-');
  if (upcase_(*s) == 'E')
    return false;

  digit = false;
  while (*s != '\0') {
    if (!(*s == 'e' || *s == 'E' || *s == '+' || *s == '-' || *s == '.' || isdigit(*s)))
      return false;
    else if (isdigit(*s))
      digit = true;
  }

  if (!digit)
    return false;
  return true;
}


static short makehexdigit(char c)
{
  switch (c) {

  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
    return c - 55;

  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
    return c - 87;

  default:
    if (isdigit(c))
      return c - '0';
    else
      return 0;
      
  }
}


long hatoi(char *s)
{
  /* "Hex-String to Integer", String wird unbedingt als Hex-Zahl interpretiert */
  long erg;

  erg = 0;
  if (*s == '$')
    s++;
  while (*s)
    erg = (erg << 4) + makehexdigit(*s++);
  return erg;
}


long batoi(char *s)
{
  /* "Bin-String to Integer", String wird unbedingt als Bin-Zahl interpretiert */
  long erg;

  erg = 0;
  if (*s == '%')
    s++;
  while (*s) {
    erg <<= 1;
    if (*s++ != '0')
      erg++;
  }
  return erg;
}


static char hextab[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

#define digits 8

void int2hstr(long i, char *s)
{
  /* "Integer to Hex-String", Zahl als Hex-Zahl dargestellt */
  register short ct;
  register char *p = hextab;

  s[digits] = '\0';
  for (ct = 1; ct <= digits; ct++) {
    s[digits - ct] = p[i & 0xf];
    i = ((unsigned long)i) >> 4;
  }
  del_lead0(s);
}

#undef digits


void int2hchar(short i, char *c1, char *c2)
{
  /* Bereich von $00..$FF  */
  short h;
  char *p;

  p = hextab;
  h = (((unsigned)i) >> 4) & 0xf;
  *c1 = p[h];
  h = i & 0xf;
  *c2 = p[h];
}


void hstr2str(char *h, char *s)
{
  sprintf(s, "%ld", hatoi(h));
}


void str2hstr(char *s, char *h)
{
  int2hstr(atoi(s), h);
}


void del_mulblanks(char *s)
{
  /* Laesst in einer Folge von Leerzeichen nur je eines stehen */
  /* schlecht kodiert, wird aber kaum benutzt */
  static char dspace[3] = "  ";
  short x = 1;

  while ((x = strpos2(s, dspace, x)) > 0) strdelete(s, x, 1);
}

short count_words(register char *s)
{
  register short      erg   = 0;
  register boolean    space = true;
  
  while (*s != '\0') {
    if (*s == ' ' || *s == tab) space = true;
    else {
      if (space) erg++;
      space = false;
    }
    s++;
  }
  return erg;
}


/* Holt naechstes Wort aus "inp" und loescht   */
/* dieses dort, Leerzeichen werden ueberlesen  */

void get_word(register char *inp, register char *outp)
{
  register char       	*p;

  p   	    = inp;
  while (*p == tab || *p == ' ') p++;
  while (*p != '\0' && *p != tab && *p != ' ') *outp++ = *p++;
  *outp     = '\0';
  while (*p == tab || *p == ' ') p++;
  while ((*inp++ = *p++));
}

/* Holt naechstes Wort aus "inp"  */
/* Leerzeichen werden ueberlesen  */

void get_pword(register char **inp, register char *outp)
{
  register char       	*p;

  p   	    = *inp;
  while (*p == tab || *p == ' ') p++;
  while (*p != '\0' && *p != tab && *p != ' ') *outp++ = *p++;
  *outp     = '\0';
  while (*p == tab || *p == ' ') p++;
  *inp	    = p;
}

/* Holt naechstes Wort aus "inp" und loescht   */
/* dieses dort, Leerzeichen werden ueberlesen  */
/* Wenn das Wort mit " beginnt, wird bis zum   */
/* nächsten " gelesen.	      	      	       */

void get_quoted(register char *inp, register char *outp)
{
  register char       	*p;

  p   	    = inp;
  while (*p == tab || *p == ' ') p++;
  if (*p == '"') {
    p++;
    while (*p != '\0' && *p != '"') *outp++ = *p++;
    if (*p) p++;  
  } else {
    while (*p != '\0' && *p != tab && *p != ' ') *outp++ = *p++;
  }
  *outp     = '\0';
  while (*p == tab || *p == ' ') p++;
  while ((*inp++ = *p++));
}

/* Holt naechstes Wort aus "inp"      	       */
/* Leerzeichen werden ueberlesen      	       */
/* Wenn das Wort mit " beginnt, wird bis zum   */
/* nächsten " gelesen.	      	      	       */

void get_pquoted(register char **inp, register char *outp)
{
  register char       	*p;

  p   	    = *inp;
  while (*p == tab || *p == ' ') p++;
  if (*p == '"') {
    p++;
    while (*p != '\0' && *p != '"') *outp++ = *p++;
    if (*p) p++;  
  } else {
    while (*p != '\0' && *p != tab && *p != ' ') *outp++ = *p++;
  }
  *outp     = '\0';
  while (*p == tab || *p == ' ') p++;
  *inp = p;
}

char *del_comment(char *z, char c)
{
  /* Loescht String nach erstem Auftreten von "C" */
  char *p;

  p = strchr(z, c);
  if (p != NULL) *p = '\0';
  return p;
}


/* --------------------------------------------------------------------- */



/*
 get_lline und put_line operieren in einem Pufferspeicher.
 das makro get_line begrenzt get_lline auf 255 zeichen lange zeilen.
 die Zeilen sollten zum Lesen mit CR+LF oder LF oder CR abgeschlossen sein
 */

#define lf '\012'
#define cr '\015'

void get_lline(register char *buf, register long *posi, long ende, register char *zeile, register short maxlen)
{
  register char *p, *e;
  
  p = &buf[*posi];
  if (ende - *posi > maxlen)
    e = &buf[*posi + maxlen];
  else
    e = &buf[ende];
  
  while (p < e && *p != lf && *p != cr) *zeile++ = *p++;
  *zeile = '\0';
  if (p < e && *p == cr) p++;
  if (p < e && *p == lf) p++;
  *posi = (long)p - (long)buf;
}


void next_line(register char *buf, register long *posi, long ende)
{
  register char *p, *e;

  p = &buf[*posi];
  e = &buf[ende];
  while (p < e && *p != lf && *p != cr) p++;
  if (p < e && *p == cr) p++;
  if (p < e && *p == lf) p++;
  *posi = (long)p - (long)buf;
}

void prev_line(register char *buf, register long *posi)
{
  register char *p;

  p = &buf[*posi];
  while (p > buf && *p != lf && *p != cr) p--;
  if (p > buf && *p == lf) p--;
  if (p > buf && *p == cr) p--;
  while (p > buf && *p != lf && *p != cr) p--;
  *posi = (long)p - (long)buf;
}

void put_line(register char *buf, register long *posi, register const char *zeile)
{
  register char *p;

  p = &buf[*posi];
  while ((*p++ = *zeile++));
  p[-1] = 10;
  *posi = (long)p - (long)buf;
}

#undef lf
#undef cr

/* dp_randomize creates a random number in limits of LOW - HIW.	*/
/* Is needed for password computations.	      	      	      	*/

short dp_randomize(short low, short hiw)
{
  static boolean  initialized = false;
  
  /* We check the initialization here and not at program start	*/
  /* because this leads to unpredictable initial values for   	*/
  /* srand()...       	      	      	      	      	      	*/
  /* The program startup is visible with "ps" and also in the   */
  /* bbs with the "VERSION" command.         	      	      	*/
  
  if (!initialized) {
    srand((int)time(NULL));
    initialized = true;
  }
  return (low + (short)((1.0*((int)(hiw-low)+1)) * rand() / (RAND_MAX+1.0)));
}
