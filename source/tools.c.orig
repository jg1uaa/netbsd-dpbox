/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/

#define TOOLS_G
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "tools.h"
#include "pastrix.h"
#include "filesys.h"
#include "crc.h"
#include "sort.h"
#include "boxglobl.h"
#include "boxlocal.h"
#include "huffman.h"
#include "main.h"
#include "md2md5.h"


/* code conversion for umlauts is partially from TNT by dl4ybg */

struct codeconv_tab {
  char code;
  char conv_code;
  char conv_code1;
  char conv_code2;
};

static struct codeconv_tab codeconv_tab[] = {
  {0x8E,0xC4,'A','e'}, /* Ae */
  {0x99,0xD6,'O','e'}, /* Oe */
  {0x9A,0xDC,'U','e'}, /* Ue */
  {0x84,0xE4,'a','e'}, /* ae */
  {0x94,0xF6,'o','e'}, /* oe */
  {0x81,0xFC,'u','e'}, /* ue */
  {0xE1,0xDF,'s','s'}, /* ss */
  {0x9E,0xDF,'s','s'}, /* ss atari */
  {0,0,0,0}
};

char conv_umlaut_from_local(char code)
{
  int i;
  
  for (i = 0; (codeconv_tab[i].conv_code != 0); i++) {
    if (code == codeconv_tab[i].conv_code) {
      return(codeconv_tab[i].code);
    }
  }
  return(code);
}

char conv_umlaut_to_local(char code)
{
  int i;
  
  for (i = 0; (codeconv_tab[i].code != 0); i++) {
    if (code == codeconv_tab[i].code) {
      return(codeconv_tab[i].conv_code);
    }
  }
  return(code);
}

void conv_string_from_local(char *s)
{
  while (*s) *s++ = conv_umlaut_from_local(*s);
}

void conv_string_to_local(char *s)
{
  while (*s) *s++ = conv_umlaut_to_local(*s);
}

boolean conv_file_umlaut(boolean to_local, char *fname)
{
  short k, l;
  char	hs[256], tmp[256];
  
  snprintf(tmp, 255, "%sDPSHD_XXXXXX", tempdir);
  mymktemp(tmp);
  l = sfcreate(tmp, FC_FILE);
  if (l < minhandle) return false;
  k = sfopen(fname, FO_READ);
  if (k < minhandle) {
    sfclosedel(&l);
    return false;
  }
  
  while (file2str(k, hs)) {
    if (to_local)
      conv_string_to_local(hs); /* umlaut conversion */
    else
      conv_string_from_local(hs); /* umlaut conversion */
    str2file(&l, hs, true);
  }

  sfclosedel(&k);
  sfclose(&l);
  sfrename(tmp, fname);
  return true;
}

/* end of code conversion *********************************/

boolean positive_arg(char *s)
{
  return (useq(s, "ON")  || useq(s, "YES") || useq(s, "+")    ||
	  useq(s, "OUI") || useq(s, "JA")  || useq(s, "TRUE") ||
	  useq(s, "AN")  || useq(s, "SI")  || useq(s, "1"));
}


/* Ruft den Watchdog auf     */
void dp_watchdog(short what, short value)
{
  static long	lastwdreset	= 0;

  long	t;

  if (what == 2) {  /*reset angefordert*/
    t			= statclock();
    if (t - lastwdreset < 5*TICKSPERSEC && t > lastwdreset)   /* 5 seconds */
      return;
    else
      lastwdreset	= t;
  } else
    lastwdreset		= 0;

  linux_watchdog(what, value);
}


/* Ruft ein externes Programm auf, Name (auch mit Pfad) in PROG, Uebergabe-  */
/* parameter in PAR                                                          */

short call_prg(char *prog, char *par, char *redir)
{
  int	status;
  char	string[256];

  if (prog[0] == '\0') return(-1);

  strcpy(string,prog);

  if (par[0] != '\0') {
      strcat(string," ");
      strcat(string,par);
  }

  if (redir[0] != '\0') {
      strcat(string," >");
      strcat(string,redir);
  }
  else strcat(string," >/dev/null");

  strcat(string," 2>&1");

  status = system(string);
#ifdef __macos__
  return (status);
#endif
#if defined(__linux__) || defined(__NetBSD__)
  return (statusconvert(status));
#endif
}


short exec_prg(char *prog, char *par, char *redir)
{
  char	string[256];

  if (prog[0] == '\0') return(-1);

  strcpy(string,prog);

  if (par[0] != '\0') {
      strcat(string," ");
      strcat(string,par);
  }

  if (redir[0] != '\0') {
      strcat(string," >");
      strcat(string,redir);
  }
  else strcat(string," >/dev/null");

  strcat(string," 2>&1");

  execl("/bin/sh","sh","-c",string, (char *) 0);

  return (0);
}



/* Ruft den LZH/GZIP auf, Ein/Ausgabe ueber Dateien s1/s2 */

short packer(boolean gzip, boolean preserve_original, boolean encode,
		char *s1, char *s2, boolean crlfconv)
{
  dp_watchdog(2, 4711);   /*watchdog resetten*/
  if (encode)
    return enchuf(gzip, preserve_original, s1, s2, crlfconv);
  else
    return dechuf(gzip, preserve_original, s1, s2, crlfconv);
}


/* Ruft den LZH/GZIP auf, Eingabe im Speicher, Ausgabe bei genug RAM ebenfalls im */
/* RAM, ansonsten in Datei f2                                                */

short mempacker(boolean gzip, boolean encode, char *mem, long size, char **rmem,
		    long *rsize, char *f2, boolean crlfconv)
{
  dp_watchdog(2, 4711);   /* watchdog resetten */
  if (encode)
    return enchufmem(gzip, mem, size, (void **)rmem, rsize, f2, crlfconv);
  else
    return dechufmem(gzip, mem, size, (void **)rmem, rsize, f2, crlfconv);
}


static long cpuspeed = 0, cpuspeed2, cattime, cpudd;

void quick_speed(void)
{
  long	x, z, erg;

  erg	= get_cpuusage();
  z	= 0;
  for (x = 1; x <= 50000L; x++)		/* 100% ca. 45	*/
    z	+= x;
  erg	= get_cpuusage() - erg;
  fast_machine = (erg < 10);
  cpudd	= z;
}

static void utc_clock_test(void);

static void calc_speed(void)
{
  long		x, z, erg;

  erg		= get_cpuusage();
  z		= 0;
  for (x = 1; x <= 100*1000000L; x++)
    z		+= x;
  cpuspeed	= get_cpuusage() - erg;
  erg		= get_cpuusage();
  for (x = 1; x <= 10*600; x++)
    utc_clock_test();
  cpuspeed2	= get_cpuusage() - erg;
  if (cpuspeed == 0)
    cpuspeed	= 1;
  if (cpuspeed2 == 0)
    cpuspeed2	= 1;
  cattime	= clock_.ixtime;
  cpudd		= z;	/* this prevents the compiler from skipping the loop (hopefully) */
}

long get_cpu_speed(short mode)
{
  if (cpuspeed == 0 || clock_.ixtime - cattime > SECPERDAY / 4)
    calc_speed();
  switch (mode) {

  case 2:
    return (47000L*10 / cpuspeed2 * 200 / TICKSPERSEC);
    break;

  default:
    return (92800L*100 / cpuspeed * 200 / TICKSPERSEC);
    break;
  }
}


boolean insecure(char *s)
{
  char hs[256];

  if (strstr(s, "../") != NULL)
    return true;
  if (strstr(s, "..*") != NULL)
    return true;
  if (strstr(s, ".?/") != NULL)
    return true;
  if (strstr(s, ".?*") != NULL)
    return true;
    
  strcpy(hs, s);
  get_path(hs);
  if (strchr(hs, '*') != NULL)
    return true;
  if (strchr(hs, '?') != NULL)
    return true;
  if (strchr(hs, '~') != NULL)
    return true;
    
  strcpy(hs, s);
  del_path(hs);
  if (strchr(hs, '~') == hs)
    return true;
    
  return false;
}


short calc_prozent(long v1, long v2)
{
  if (v2 > 100000L)
    return (v1 / (v2 / 100));
  else if (v2 > 0)
    return (v1 * 100 / v2);
  else
    return 0;
}


void expand_tabs(char *hs, short tabsize)
{
  short x, z;
  char s1[256];

  do {
    x = strpos2(hs, "\t", 1);
    if (x > 0) {
      strdelete(hs, x, 1);
      z = x;
      while (z % tabsize != 0)
	z++;
      z++; 
      *s1 = '\0';
      lspacing(s1, z - x);
      strinsert(s1, hs, x);
    }
  } while (x != 0);
}


static void process_backspaces(char *hs)
{
  short x, l;

  l = strlen(hs);
  if (l < 1 || l > 255)
    return;
  x = 1;
  while (x < l) {
    if (hs[x - 1] != '\b') {
      x++;
      continue;
    }
    strdelete(hs, x, 1);
    l--;
    if (x > 1) {
      strdelete(hs, x - 1, 1);
      l--;
      x--;
    }
  }
}


boolean give_cookie(short x, fdoutproctype outproc, char *fname)
{
  long fs, a, b, sp, faktor;
  short k, y, lc;
  char delimiter;
  boolean ok;
  char hs[256];

  fs = sfsize(fname);
  if (fs <= 0)
    return false;
  
  if (fs >= SHORT_MAX) {
    faktor = (fs / SHORT_MAX);
    if (faktor >= SHORT_MAX)
      faktor = SHORT_MAX - 1;  
    do {
      a = (long)dp_randomize(0, faktor);
      b = (long)dp_randomize(0, SHORT_MAX - 1);
      sp = a * SHORT_MAX + b;
    } while (sp < 0 || sp >= fs);
  }
  else
    sp = (long)dp_randomize(0, fs);
    
  k = sfopen(fname, FO_READ);
  if (k < minhandle)
    return false;

  ok = false;
  lc = 0;
  do {
    lc++;
    if (sfseek(sp, k, SFSEEKSET) == sp) {
      y = 0;
      while (file2str(k, hs) && hs[0] != '-' && hs[0] != '%' && y < 400)
	y++;

      if (hs[0] == '-')
        delimiter = '-';
      else if (hs[0] == '%')
        delimiter = '%';
      else
        delimiter = '\0';

      if (delimiter != '\0') {
        y = 0;
	while (file2str(k, hs) && hs[0] != delimiter && y < 400) {
	  y++;
	  process_backspaces(hs);
	  expand_tabs(hs, 8);
	  outproc(x, hs);
	  ok = true;
	}
      }
      
    }
    
    if (!ok)
      sp = 0;
      
  } while (!(ok || lc > 1));
  
  sfclose(&k);
  
  return ok;
}


void calc_ixsecs_to_string(time_t l, char *hs)
{
  long d, h, m, s;

  d = l / SECPERDAY;
  l -= d * SECPERDAY;
  h = l / 3600;
  l -= h * 3600;
  m = l / 60;
  s = l - m * 60;
  sprintf(hs, "%ldd, %.2ld:%.2ld:%.2ld", d, h, m, s);
}


void file_delete(short x, char *cmd, fdoutproctype outproc)
{
  DTA dirinfo;
  short result, ct, l;
  char pa[256], dn[256], df[256];
  char STR1[256];

  ct = 0;
  strcpy(pa, cmd);
  if (insecure(pa)) {
    if (outproc != NULL)
      outproc(x, "invalid path");
  } else {
    sprintf(pa, "%s%s", boxdir, strcpy(STR1, pa));
    l = strlen(boxdir);
    strcpy(df, pa);
    del_path(df);
    get_path(pa);
    sprintf(STR1, "%s%s", pa, df);
    result = sffirst(STR1, 0, &dirinfo);   /* alle normalen Files suchen */
    while (result == 0) {
      sprintf(dn, "%s%s", pa, dirinfo.d_fname);
      sfdelfile(dn);
      ct++;
      strdelete(dn, 1, l);
      if (x >= 0)
        if (outproc != NULL)
  	  outproc(x, dirinfo.d_fname);
      result = sfnext(&dirinfo);
    }
  }
  if (ct <= 0) {
    if (outproc != NULL)
      outproc(x, "no matching file found");
    return;
  }
  sprintf(dn, "%d", ct);
  strcat(dn, " file");
  if (ct != 1)
    strcat(dn, "s");
  strcat(dn, " deleted");
  if (x >= 0)
    if (outproc != NULL)
      outproc(x, dn);
}


#ifdef HAS_NO_STRCASECMP
boolean useq(char *s1, char *s2)
/* stolen from p2c.c of David Gillespie */
{
    char c1, c2;

    while (*s1) {
	if (*s1++ != *s2++) {
	    if (!s2[-1])
		return false;
	    c1 = upcase_(s1[-1]);
	    c2 = upcase_(s2[-1]);
	    if (c1 != c2)
		return false;
	}
    }
    if (*s2)
	return false;
    return true;
}
#endif

short uspos(char *s1, char *s2)
{
  char c;
  char *tmp;
  char b1[256], b2[256];

  tmp = b1;
  while ((c = *s1++)) *tmp++ = upcase_(c);
  *tmp = '\0';
  tmp = b2;
  while ((c = *s2++)) *tmp++ = upcase_(c);
  *tmp = '\0';
  return strpos2(b2, b1, 1);
}


static void spdputs(short d, short m, short y, char sep, char *s)
{
  sprintf(s, "%.2d%c%.2d%c%.2d", d, sep, m, sep, y);
}


static void conc_date(boolean datum, char sep, unsigned short date, char *s)
{
  short d, m, y;

  if (datum)
    Get_DDate(date, &d, &m, &y);
  else
    Get_DTime(date, &d, &m, &y);
  spdputs(d, m, y % 100, sep, s);
}


static void make_sortdate(unsigned short date, unsigned short time, char *s)
{
  sprintf(s, "%.10ld", dos2ixtime(date, time));
}


static void make_sortsize(long size, char *s)
{
  sprintf(s, "%.10ld", size);
}


static void make_sortext(char *ins, char *s)
{
  get_ext(ins, s);
  rspacing(s, 10);
  cut(s, 10);
}


static boolean shorten_dirstring(char *hs, short maxct)
{
  short x, y, todel;

  x = strlen(hs);
  if (x <= maxct)
    return false;

  todel = x - maxct + 2;
  y = 1;
  while (y < x && hs[y - 1] != extsep)
    y++;
  if (y > todel + 2) {
    strdelete(hs, y - todel + 1, todel);
    strinsert("..", hs, y - todel + 1);
  } else {
    cut(hs, maxct - 2);
    strcat(hs, "..");
  }
  return true;
}


#define sbufsize        32768*5
#define SBNAME          0
#define SBDATE          1
#define SBSIZE          2
#define SBEXT           3

void file_dir(boolean supervisor, char *path, char *options, short x,
	      fdoutproctype outproc)
{

  DTA dirinfo;
  short result, l;
  boolean longdir, xdir, fnums, count, shortened;
  short sortby, countfor;
  char *sbuf, *longname;
  short maxc, sct;
  long sbs, rbs;
  short h, ct, cct;
  char hs[256], dn[256], s[256], ds[256], out[256], pn[256];

  sbuf = malloc(sbufsize);
  if (sbuf == NULL)
    return;

  sbs = 0;
  maxc = 0;
  count = false;
  countfor = 0;

  strcpyupper(ds, options);
  
  h = strpos2(ds, "-C", 1);
  if (h > 0) {
    strcpy(s, ds);
    strdelete(s, 1, h + 1);
    get_word(s, pn);
    countfor = atoi(pn);
    count = true;
  }

  if (strstr(ds, "-DATE") != NULL)
    sortby = SBDATE;
  else if (strstr(ds, "-SIZE") != NULL)
    sortby = SBSIZE;
  else if (strstr(ds, "-EXT") != NULL)
    sortby = SBEXT;
  else
    sortby = SBNAME;

  xdir = (count || (strstr(ds, "-X") != NULL && x >= 0));
  longdir = (xdir || strstr(ds, "-L") != NULL);

  strcpy(pn, path);
  get_path(pn);

  if (!count) {
    if (x >= 0 && strstr(ds, "-I") != NULL) {
      sprintf(hs, "%s.index", pn);
      h = sfopen(hs, FO_READ);
      if (h >= minhandle) {
	while (file2str(h, hs))
	  outproc(x, hs);
	sfclose(&h);
	outproc(x, "");
      }
    }

    result = sffirst(path, 16, &dirinfo);   /* alle Subdirectories suchen */
    while (result == 0) {
      if (dirinfo.d_attrib == 16)
      {  /* es handelt sich um einen Directoryeintrag */
	strcpy(hs, dirinfo.d_fname);
	if (longdir) {
	  shorten_dirstring(hs, 24);
	  sprintf(out, "  * %s/", hs);
	  rspacing(out, 29);
	  strcat(out, "           ");
	  conc_date(true, '/', dirinfo.d_date, s);
	  strcat(out, s);
	  conc_date(false, ':', dirinfo.d_time, s);
	  strcat(out, "  ");
	  strcat(out, s);
	} else {
	  sprintf(out, "%s/", hs);
	  l = strlen(out);
	  if (l > maxc)
	    maxc = l;
	}

	switch (sortby) {

	case SBDATE:
	  make_sortdate(dirinfo.d_date, dirinfo.d_time, ds);
	  break;

	case SBSIZE:
	  make_sortsize(dirinfo.d_length, ds);
	  break;

	case SBEXT:
	  make_sortext(dirinfo.d_fname, ds);
	  break;

	default:
	  *ds = '\0';
	  break;
	}
	sprintf(hs, "%s%s", ds, out);
	if (sbs + strlen(hs) + 1 < sbufsize)
	  put_line(sbuf, &sbs, hs);
      }
      result = sfnext(&dirinfo);
    }

    if (longdir) {
      sort_mem(sbuf, &sbs, false);
      rbs = 0;
      while (rbs < sbs) {
	get_line(sbuf, &rbs, sbs, hs);
	if (sortby != SBNAME)
	  outproc(x, &hs[10]);
	else
	  outproc(x, hs);
      }

      if (xdir && sbs > 0)
	outproc(x, "");

      sbs = 0;
    }

  }

  result = sffirst(path, 0, &dirinfo);   /* alle normalen Files suchen */
  while (result == 0) {
    if (dirinfo.d_attrib != 16) {
      strcpy(dn, dirinfo.d_fname);
      if ((supervisor && !xdir) || dn[0] != '.') {
	if (longdir) {
	  if (!count) shortened = shorten_dirstring(dn, 25);
	  else shortened = false;
	  rspacing(dn, 25);
	  sprintf(hs, "%ld", dirinfo.d_length);
	  lspacing(hs, 8);
	  sprintf(out, "    %s %s", dn, hs);
	  rspacing(out, 40);
	  conc_date(true, '/', dirinfo.d_date, s);
	  strcat(out, s);
	  conc_date(false, ':', dirinfo.d_time, s);
	  strcat(out, "  ");
	  strcat(out, s);
	  if (shortened) {
	    sprintf(s, "?/%s", dirinfo.d_fname);
	    if (strlen(out) + strlen(s) < 256) strcat(out, s);
	  }
	} else {
	  strcpy(out, dn);
	  l = strlen(out);
	  if (l > maxc)
	    maxc = l;
	}

	switch (sortby) {

	case SBDATE:
	  make_sortdate(dirinfo.d_date, dirinfo.d_time, ds);
	  break;

	case SBSIZE:
	  make_sortsize(dirinfo.d_length, ds);
	  break;

	case SBEXT:
	  make_sortext(dirinfo.d_fname, ds);
	  break;

	default:
	  *ds = '\0';
	  break;
	  
	}
	sprintf(hs, "%s%s", ds, out);

	if (sbs + strlen(hs) + 1 < sbufsize)
	  put_line(sbuf, &sbs, hs);

      }
    }
    result = sfnext(&dirinfo);
  }

  sort_mem(sbuf, &sbs, false);

  rbs = 0;
  sct = 0;

  if (!longdir) {
    *s = '\0';
    if (maxc > 78)
      maxc = 78;
  }

  cct = 0;
  fnums = (xdir && strpos2(path, (sprintf(out, "%c%c%c%c", dirsep, allquant, extsep,
				  allquant), out), 1) == strlen(path) - 3);

  while (rbs < sbs) {
    get_line(sbuf, &rbs, sbs, hs);
    if (sortby != SBNAME)
      strdelete(hs, 1, 10);
    if (!longdir) {
      rspacing(hs, maxc);
      if (sct + maxc > 78) {
	outproc(x, s);
	sct = maxc;
	strcpy(s, hs);
	continue;
      }
      if (sct == 0) {
	strcpy(s, hs);
	sct = maxc;
      } else {
	sprintf(s + strlen(s), "  %s", hs);
	sct += maxc + 2;
      }
      continue;
    }

    if (count) {
      cct++;
      if (cct == countfor) {
	strdelete(hs, 1, 4);
	get_word(hs, path);
	free(sbuf);
	return;
      }
      continue;
    }

    if (fnums) {
      cct++;
      strdelete(hs, 1, 3);
      sprintf(out, "%d", cct);
      lspacing(out, 3);
      strcat(out, hs);
      strcpy(hs, out);
    }

    longname = strstr(hs, "?/");
    if (longname != NULL) {
      *longname = '\0';
      longname++;
      longname++;
    }

    outproc(x, hs);

    if (!xdir)
      continue;

    if (longname != NULL && *longname) {
      strcpy(s, longname);
    } else {
      strdelete(hs, 1, 4);
      get_word(hs, s);
    }
    if (s[0] == '.')
      continue;

    sprintf(hs, "%s.%s", pn, s);
    h = sfopen(hs, FO_READ);
    if (h < minhandle)
      continue;

    for (ct = 1; ct <= 4; ct++) {
      strcpy(hs, "    ");
      if (file2str(h, &hs[4])) {
	outproc(x, hs);
      }
    }
    sfclose(&h);
    outproc(x, "");
  }

  if (!longdir && sct > 0)
    outproc(x, s);

  if (count)
    *path = '\0';
  free(sbuf);
}

#undef sbufsize
#undef SBNAME
#undef SBDATE
#undef SBSIZE
#undef SBEXT


short filecp(char *filea, char *fileb, char *root, boolean del_source,
	     short x, fdoutproctype outproc)
{
  short Result;
  short err;
  boolean dest_set_dir, dest_is_dir;
  DTA dirinfo;
  short result, loopct;
  char fpath[256], fa[256], fb[256], fx[256];
  Result = -1;
  err = 0;

  if (*root != '\0') {
    if (insecure(filea))
      err = -1;
    else if (insecure(fileb))
      err = -1;
    else if (strstr(filea, root) != filea)
      err = -1;
    else if (strstr(fileb, root) != fileb)
      err = -1;

    if (err != 0) {
      if (x >= 0)
	outproc(x, "no permission");
      return Result;
    }
  }

  strcpy(fx, fileb);
  if (fx[strlen(fx) - 1] == dirsep) {
    strdelete(fx, strlen(fx), 1);
    dest_set_dir = true;
  } else
    dest_set_dir = false;

  dest_is_dir = (sffirst(fx, 16, &dirinfo) == 0 && dirinfo.d_attrib == 16);

  if (dest_set_dir && !dest_is_dir) {
    if (x >= 0)
      outproc(x, "destination is not a directory");
    return Result;
  }

  strcpy(fpath, filea);
  get_path(fpath);

  if (dest_is_dir) {
    sprintf(fb, "%s%c", fx, dirsep);
    if (!strcmp(fb, fpath)) {
      if (x >= 0)
	outproc(x, "invalid operation");
      return Result;
    }
  }

  loopct = 0;
  result = sffirst(filea, 0, &dirinfo);
  while (result == 0 && (loopct == 0 || dest_is_dir)) {
    if (dirinfo.d_attrib != 16) {
      sprintf(fa, "%s%s", fpath, dirinfo.d_fname);
      if (dest_is_dir)
	sprintf(fb, "%s%c%s", fx, dirsep, dirinfo.d_fname);
      else
	strcpy(fb, fx);

      if (x >= 0)
	outproc(x, dirinfo.d_fname);

      if (del_source)
	filemove(fa, fb);
      else
	filecopy(fa, fb);

      if (dest_is_dir)
	Result = 1;
      else
	Result = 0;
      loopct++;
    }
    result = sfnext(&dirinfo);
  }
  return Result;
}


/* Errechnet eine Checksumme ueber einen String       */

char calccs(char *s)
{
  char erg;

  erg = 0;

  while (*s != '\0')
    erg += *s++;

  return erg;
}

/* Kopiert und errechnet eine Checksumme ueber einen String       */

char strcpycs(register char *s2, register char *s1)
{
  register char erg;
  register char ch;

  erg = 0;

  do {
    ch = *s1++;
    erg += ch;
    *s2++ = ch;
  } while (ch != '\0');

  return erg;
}


#define blocksize       8192L

/* Errechnet eine CRC ueber ein File                 */

unsigned short file_crc(short methode, char *name, unsigned short preload,
			long start, long size)
{
  unsigned short crc;
  short k;
  long fsize, done, cstep, psize, hsize;
  char *buf;

  crc = preload;
  fsize = sfsize(name);

  if (size > 0)
    fsize = size;
  if (start > 0) {
    if (size == 0)
      fsize -= start;
  }

  if (fsize <= 0)
    return crc;

  psize = blocksize;
  if (psize > fsize)
    psize = fsize;
  buf = malloc(psize);
  if (buf == NULL) {
    hsize = maxavail__();
    if (hsize > 0) {
      if (hsize > psize)   /* darf nicht vorkommen */
	hsize = psize;
      psize = hsize;
      buf = malloc(psize);
    }
  }
  if (buf == NULL)
    return crc;

  k = sfopen(name, FO_READ);
  if (k >= minhandle) {
    if (start > 0)
      sfseek(start, k, SFSEEKSET);
    done = 0;
    cstep = 1;
    while (done < fsize && cstep > 0) {
      cstep = sfread(k, psize, buf);
      if (cstep <= 0)
	break;
      done += cstep;

      switch (methode) {

      case 0:
	crcfcs_buf(buf, cstep, &crc);
	break;

      case 1:
	crc_16_buf(buf, cstep, &crc);
	break;

      case 2:
	crcthp_buf(buf, cstep, &crc);
	break;

      case 3:
	crcfbb_buf(buf, cstep, &crc);
	break;

      case 4:
	checksum8_buf(buf, cstep, &crc);
	break;

      case 5:
	checksum16_buf(buf, cstep, &crc);
	break;

      }
    }
    sfclose(&k);
  }
  free(buf);
  return crc;
}

#undef blocksize


void wochentag(char *lan, short d, short m, short y, char *s)
{
  char hs[256];

  get_language(week_day(d, m, y) + 66, langmemroot, lan, hs);
  cut(hs, 10);
  strcpy(s, hs);
}


void unhpath(char *ins, char *outs)
{
  short	      k, i;

  k   	      = strpos2(ins, ".", 1);
  i   	      = strpos2(ins, "-", 1);
  if (k > 0)
    k--;
  else
    k 	      = strlen(ins);
  if (k > LEN_CALL)
    k 	      = LEN_CALL;
  if (i > 0 && i <= k)
    k 	      = i - 1;

  if (ins != outs) strncpy(outs, ins, k);
  outs[k]     = '\0';
}


void del_tnc3port(char *callin, char *callout)
{
  if (callin[1] == ':')
    strsub(callout, callin, 3, strlen(callin) - 2);
  else
    strncpy(callout, callin, 9);
  cut(callout, 9);
}


void del_callextender(char *callin, char *callout)
{
  char *p;
  char h[256];

  strcpy(h, callin);
  p = strchr(h, '-');
  if (p != NULL) *p = '\0';
  if (h[1] == ':')
    strdelete(h, 1, 2);
  cut(h, LEN_CALL);
  strcpy(callout, h);
}


void get_callextender(char *callin, char *callout)
{
  char *p;

  p = strchr(callin, '-');
  if (p == NULL) {
    callout[0] = '\0';
    return;
  }
  strncpy(callout, ++p, 2);
  cut(callout, 2);
}

/* check for correct input of "callsyntax" */
boolean check_other_callsyntax(char *syntax)
{
  int 	  len;
  boolean error = false;
  char	  *p;
  
  if (*syntax == '\0') return true;
  
  upper(syntax);
  
  len = 0;
  p   = syntax;
  
  do {
    switch (*p) {
      case 'A'	: len++;
      	      	  if (len > LEN_CALL) error = true;
      	      	  break;
      case 'N'	: len++;
      	      	  if (len > LEN_CALL) error = true;
      	      	  break;
      case ' '	: len = 0;
      	      	  break;
      case ','	: *p = ' ';
      	      	  len = 0;
      	      	  break;
      default 	: error = true;
      	      	  break;
    }
  } while (*++p);
  
  if (error) {
    *syntax = '\0';
    return false;
  }
  
  del_blanks(syntax);
  del_mulblanks(syntax);
  return true;
}


static short iscalpha(char c)
{
  if (isupper(c))
    return 1;
  else if (isdigit(c))
    return 0;
  else if (islower(c))
    return 1;
  else
    return -1;
}

/* translate callsign into syntax pattern (A,N) */
static void normalize_call(char *inp, char *outp)
{
  while (*inp) {
    switch (iscalpha(*inp++)) {
      case 1: *outp++ = 'A';
      	      break;
      case 0: *outp++ = 'N';
      	      break;
      default:*outp++ = '!';
      	      break;
    }
  }
  *outp = '\0';
}

/* check additional syntaxes (CB, MARS) */
static boolean compare_other_callsyntax(char *norm)
{
  char	w[LEN_CALL+1]; /* limitation OK, word length is checked in check_other_callsyntax */
  char  *p;

  p   = other_callsyntax;

  get_pword(&p, w);
  while (*w != '\0') {
    if (!strcmp(w, norm))
      return true;
    get_pword(&p, w);
  }
  return false;
}

/* callsign check of dpbox */
boolean callsign(char *input)
{
  short l, i;
  char	norm[LEN_CALL+1];

  /* check length for boundaries */
  l = strlen(input);
  if (l < 4 || l > LEN_CALL)
    return false;
  
  /* translate callsign into syntax pattern (A,N) */
  normalize_call(input, norm);

  /* if additional syntaxes (CB, MARS) are defined, check them first */
  if (*other_callsyntax != '\0') {
    if (compare_other_callsyntax(norm))
      return true;
  }

  /* check for amateur radio syntax */
  if (	 !strncmp(norm, "AANA", 4)
      || !strncmp(norm, "NANA", 4)
      || !strncmp(norm, "ANAA", 4)
      || !strncmp(norm, "ANNA", 4) ) {

    /* last two chars (if available) have to be alphabetical characters */
    i = 4;
    while (i < l) {
      if (norm[i++] != 'A')
      	return false;
    }

    /* OK, this is a callsign */
    return true;    
  }
  
  /* check for those REQxxx Servers */
  /* as THEBOX got really rare, we could omit that check in the future */
  /* (it only reverses type P -> B changes by THEBOX) */
  if (!strncmp(input, "REQ", 3) )
    return true;
  
  return false;
}

/* accepts hierarchical callsigns */
boolean hcallsign(char *input)
{
  calltype  call;
  
  unhpath(input, call);
  return callsign(call);
}



void Gettimestr(char *ts)
{
  char	sep;

  sep	= *ts;
  if (sep == '\0') sep = ':';
  sprintf(ts, "%.2d%c%.2d%c%.2d", clock_.hour, sep, clock_.min, sep, clock_.sec);
  cut(ts, 8);
}


void GetDatestr(char *ds)
{
  short	y;
  char	sep;

  sep	= *ds;
  if (sep == '\0') sep = '.';
  y	= clock_.year % 100; 
  sprintf(ds, "%.2d%c%.2d%c%.2d", clock_.day, sep, clock_.mon, sep, y);
  cut(ds, 8);
}


short week_day(short d, short m, short y)
{
  /* Wochentags-Berechnung (1583-4139) */

  if (y < 1583)
    y	+= 1900;
  if (m < 3) {
    m	+= 12;
    y--;
  }
  return ((d + (m * 13 + 3) / 5 + (((unsigned)(y * 5)) >> 2) - y / 100 + y / 400) % 7 + 1);
}

unsigned short Make_DTime(short Hour, short Min, short Sec)
{
  return ((Hour << 11) | (Min << 5) | (((unsigned)Sec) >> 1));
}


unsigned short Make_DDate(short Day, short Mon, short Yr)
{
  short yh;

  if (Yr >= 1980) {
    yh = Yr - 1980;
    return ((yh << 9) | (Mon << 5) | Day);
  }
  if (Yr < 80)
    yh = Yr + 20;   /* 2000 - 2079 */
  else
    yh = Yr - 80;
  return ((yh << 9) | (Mon << 5) | Day);
}


void Get_DDate(unsigned short ddate, short *Day, short *Mon, short *Yr)
{
  *Day	= ddate & 31;
  *Mon	= (ddate >> 5) & 15;
  *Yr	= ((ddate >> 9) & 127) + 80;
}


void Get_DTime(unsigned short dtime, short *Hour, short *Min, short *Sec)
{
  *Sec	= (dtime & 31) << 1;
  *Min	= (dtime >> 5) & 63;
  *Hour	= (dtime >> 11) & 31;
}


void datum22string(short day, short mon, short year, char *datestring)
{
  sprintf(datestring, "%.2d.%.2d.%.2d", day, mon, year % 100);
  cut(datestring, 8);
}

void datum22string4(short day, short mon, short year, char *datestring)
{
  sprintf(datestring, "%.2d.%.2d.%.4d", day, mon, year+1900);
  cut(datestring, 10);
}

void datum2string(unsigned short datum, char *datestring)
{
  short day, mon, year;

  Get_DDate(datum, &day, &mon, &year);
  datum22string(day, mon, year, datestring);
}

void ixdatum2string(time_t datum, char *datestring)
{
  short d, m, y, h, min, s;

  decode_ixtime(datum, &d, &m, &y, &h, &min, &s);
  datum22string(d, m, y, datestring);
}

void ixdatum2string4(time_t datum, char *datestring)
{
  short d, m, y, h, min, s;

  decode_ixtime(datum, &d, &m, &y, &h, &min, &s);
  datum22string4(d, m, y, datestring);
}

void zeit22string(short hour, short min, short sec, char *timestring)
{
  sprintf(timestring, "%.2d:%.2d:%.2d", hour, min, sec);
  cut(timestring, 8);
}

void zeit2string(unsigned short zeit, char *timestring)
{
  short hour, min, sec;

  Get_DTime(zeit, &hour, &min, &sec);
  zeit22string(hour, min, sec, timestring);
}

void dosdt2string(unsigned short date, unsigned short time, char *timestring)
{
  char w[30];
  
  datum2string(date, timestring);
  zeit2string(time, w);
  strcat(timestring, " ");
  strcat(timestring, w);
}

void ixzeit2string(time_t zeit, char *timestring)
{
  short d, m, y, h, min, s;

  decode_ixtime(zeit, &d, &m, &y, &h, &min, &s);
  zeit22string(h, min, s, timestring);
}

void ix2string(time_t zeit, char *timestring)
{
  short d, m, y, h, min, s;
  char hs[256];

  decode_ixtime(zeit, &d, &m, &y, &h, &min, &s);
  zeit22string(h, min, s, hs);
  datum22string(d, m, y, timestring);
  strcat(timestring, " ");
  strcat(timestring, hs);
}

void ix2string4(time_t zeit, char *timestring)
{
  short d, m, y, h, min, s;
  char hs[256];

  decode_ixtime(zeit, &d, &m, &y, &h, &min, &s);
  zeit22string(h, min, s, hs);
  datum22string4(d, m, y, timestring);
  strcat(timestring, " ");
  strcat(timestring, hs);
}


unsigned short str2datum(char *datestring)
{
  short day, mon, year, x;
  boolean reverse;
  char sep;
  char hs[256];
  char STR1[256];
  
  reverse = false;
  x = strpos2(datestring, ".", 1);
  if (x == 0) {
    x = strpos2(datestring, "/", 1);
    if (x == 0) {
      x = strpos2(datestring, "-", 1);
      if (x == 3) {
	reverse = true;
	sep = '-';
      } else
	sep = '\0';
    } else
      sep = '/';
  } else
    sep = '.';
  if ((unsigned)x < 32 && ((1L << x) & 0xc) != 0) {
    sprintf(hs, "%.*s", x - 1, datestring);
    day = atoi(hs);
    strdelete(datestring, 1, x);
    sprintf(STR1, "%c", sep);
    x = strpos2(datestring, STR1, 1);
    if ((unsigned)x < 32 && ((1L << x) & 0xc) != 0) {
      sprintf(hs, "%.*s", x - 1, datestring);
      mon = atoi(hs);

      strdelete(datestring, 1, x);
      sprintf(STR1, "%.2s", datestring);
      if (!strcmp(STR1, "19"))
	strsub(hs, datestring, 3, 2);
      else
	sprintf(hs, "%.2s", datestring);
      year = atoi(hs);
      if (year >= 80 && year <= 99) {
	if (reverse) {
	  x = day;
	  day = mon;
	  mon = x;
	}
	if (mon > 12 && day <= 12) {
	  x = day;
	  day = mon;
	  mon = x;
	}
	if ((unsigned)mon < 32 && ((1L << mon) & 0x1ffe) != 0 &&
	    (unsigned)day < 32 && ((1L << day) & 0xfffffffe) != 0)
	  return (Make_DDate(day, mon, year));
	else
	  return 0;
      } else
	return 0;
    } else
      return 0;
  } else
    return 0;
}


unsigned short str2zeit(char *timestring)
{
  short hour, min, sec, x;
  char sep;
  char hs[256];
  char STR1[256];
  
  x = strpos2(timestring, ":", 1);
  if (x == 0) {
    x = strpos2(timestring, ".", 1);
    if (x == 0) {
      x = strpos2(timestring, "/", 1);
      if (x == 0) {
	x = strpos2(timestring, "-", 1);
	if (x == 3)
	  sep = '-';
	else
	  sep = '\0';
      } else
	sep = '/';
    } else
      sep = '.';
  } else
    sep = ':';

  if ((unsigned)x < 32 && ((1L << x) & 0xc) != 0) {
    sprintf(hs, "%.*s", x - 1, timestring);
    hour = atoi(hs);
    strdelete(timestring, 1, x);
    sprintf(STR1, "%c", sep);
    x = strpos2(timestring, STR1, 1);
    if (x == 0)
      x = 3;
    if ((unsigned)x < 32 && ((1L << x) & 0xc) != 0) {
      sprintf(hs, "%.*s", x - 1, timestring);
      min = atoi(hs);
      strdelete(timestring, 1, x);
      if (strlen(timestring) >= 2) {
	sprintf(hs, "%.2s", timestring);
	sec = atoi(hs);
      } else
	sec = 0;
      if ((unsigned)hour < 32 && ((1L << hour) & 0xffffffL) != 0 &&
	  (unsigned)min <= 59 && (unsigned)sec <= 59)
	return (Make_DTime(hour, min, sec));
      else
	return 0;
    } else
      return 0;
  } else
    return 0;

}


/******************************************************************** */

time_t daydiff(unsigned short date1, unsigned short time1, unsigned short date2,
	     unsigned short time2)
{
/* unused */
  short d1, m1, y1, h1, min1, s1, d2, m2, y2, h2, min2, s2;

  Get_DDate(date1, &d1, &m1, &y1);
  Get_DDate(date2, &d2, &m2, &y2);
  Get_DTime(time1, &h1, &min1, &s1);
  Get_DTime(time2, &h2, &min2, &s2);
  return calc_ixtime(d2, m2, y2, h2, min2, s2) - calc_ixtime(d1, m1, y1, h1, min1, s1);
}

static time_t ix_ytable[68] = {
            0,   31536000L,   63072000L,   94694400L,  126230400L,  157766400L,
   189302400L,  220924800L,  252460800L,  283996800L,  315532800L,  347155200L,
   378691200L,  410227200L,  441763200L,  473385600L,  504921600L,  536457600L,
   567993600L,  599616000L,  631152000L,  662688000L,  694224000L,  725846400L,
   757382400L,  788918400L,  820454400L,  852076800L,  883612800L,  915148800L,
   946684800L,  978307200L, 1009843200L, 1041379200L, 1072915200L, 1104537600L,
  1136073600L, 1167609600L, 1199145600L, 1230768000L, 1262304000L, 1293840000L,
  1325376000L, 1356998400L, 1388534400L, 1420070400L, 1451606400L, 1483228800L,
  1514764800L, 1546300800L, 1577836800L, 1609459200L, 1640995200L, 1672531200L,
  1704067200L, 1735689600L, 1767225600L, 1798761600L, 1830297600L, 1861920000L,
  1893456000L, 1924992000L, 1956528000L, 1988150400L, 2019686400L, 2051222400L,
  2082758400L, 2114380800L
};

static time_t ix_mtable[12] = {
          0,  2678400L,  5097600L,  7776000L, 10368000L, 13046400L,
  15638400L, 18316800L, 20995200L, 23587200L, 26265600L, 28857600L
};


time_t calc_ixtime(short d, short m, short y, short h, short mi, short s)
{
  time_t erg;

  if (y < 70 || y > 137)
    return 0;

  if (m < 1 || m > 12)
    return 0;

  erg = ix_ytable[y - 70] + ix_mtable[m - 1] + ((time_t)d - 1) * SECPERDAY +
	(time_t)h * 3600 + (time_t)mi * 60 + (time_t)s;

  if (m > 2 && (y & 3) == 0)
    erg += SECPERDAY;

  return erg;
}


void decode_ixtime(time_t ix, short *d, short *m, short *y, short *h,
		   short *min, short *s)
/*
{
  struct tm tms;

  tms		= gmtime(ix);
  *s		= tms.sec;
  *min		= tms.min;
  *h		= tms.hour;
  *d		= tms.mday;
  *m		= tms.mon;
  *y		= tms.year;
}
*/
{
  time_t zsum, offset;

  if (ix > 0) {

    *y = 137;
    while (ix_ytable[*y - 70] > ix)
      (*y)--;

    zsum = ix - ix_ytable[*y - 70];
    *m = 12;

    if (zsum > 5097600L && ((*y) & 3) == 0) {
      zsum -= SECPERDAY;
      offset = SECPERDAY;
    } else {
      offset = 0;
    }

    while (ix_mtable[*m - 1] > zsum)
      (*m)--;

    if (*m == 2)
      zsum += offset - ix_mtable[*m - 1];
    else
      zsum -= ix_mtable[*m - 1];

    *d = zsum / SECPERDAY + 1;

    zsum %= SECPERDAY;

    *h = zsum / 3600;
    *min = zsum % 3600 / 60;
    *s = zsum % 60;

  } else {

    *d = 1;
    *m = 1;
    *y = 70;
    *h = 0;
    *min = 0;
    *s = 0;
    
  }
}

time_t calcfdate(unsigned short date, unsigned short time)
{
  short d, m, y, h, min, s;

  Get_DDate(date, &d, &m, &y);
  Get_DTime(time, &h, &min, &s);
  return (calc_ixtime(d, m, y, h, min, s));
}


time_t dos2ixtime(unsigned short ddate, unsigned short dtime)
{
  return (calcfdate(ddate, dtime));
}


void ix2dostime(time_t ix, unsigned short *ddate, unsigned short *dtime)
{
  short d, m, y, h, min, s;

  decode_ixtime(ix, &d, &m, &y, &h, &min, &s);
  *ddate = Make_DDate(d, m, y);
  *dtime = Make_DTime(h, min, s);
}


time_t string2ixtime(char *ds, char *ts)
{
  return (dos2ixtime(str2datum(ds), str2zeit(ts)));
}


time_t utc_clock(void)
{
  static time_t   last_ixtime = 0;

  if ((clock_.ixtime = sys_ixtime()) == last_ixtime)
    return clock_.ixtime;

  last_ixtime 	  = clock_.ixtime;

  decode_ixtime(clock_.ixtime, &clock_.day, &clock_.mon, &clock_.year, &clock_.hour, &clock_.min, &clock_.sec);
  zeit22string(clock_.hour, clock_.min, clock_.sec, clock_.zeit);

  clock_.year4	  = clock_.year + 1900;

  datum22string(clock_.day, clock_.mon, clock_.year, clock_.datum);
  datum22string4(clock_.day, clock_.mon, clock_.year, clock_.datum4);
  
  clock_.daystart = calc_ixtime(clock_.day, clock_.mon, clock_.year, 0, 0, 0);
  clock_.weekday  = week_day(clock_.day, clock_.mon, clock_.year);

  return clock_.ixtime;
}


static void utc_clock_test(void)
{
  static char clock2wochentag[2];
  unsigned short ddate, dtime;
  clocktype clock2;
  
  /* nur noch zum Testen der CPU-Speed ... */
  clock2.ixtime = sys_ixtime();
  decode_ixtime(clock2.ixtime, &clock2.day, &clock2.mon, &clock2.year,
		&clock2.hour, &clock2.min, &clock2.sec);
  ddate = Make_DDate(clock2.day, clock2.mon, clock2.year);
  dtime = Make_DTime(clock2.hour, clock2.min, clock2.sec);
  datum2string(ddate, clock2.datum);
  zeit2string(dtime, clock2.zeit);

  *clock2wochentag = '\0';
}

void get_language(short nr, languagetyp *root, char *which, char *s)
{
  languagetyp *lptr;
  boolean found;
  long rp;
  char w2[9];

  if (nr <= 0 || nr > MAXLANGUAGELINEDEFS) {
    strcpy(s, "error: invalid line number");
    return;
  }

  sprintf(w2, "%.3s", which);
  upper(w2);

  if (root == NULL) {
    strcpy(s, "error: no languages loaded");
    return;
  }

  do {

    found = false;
    lptr = root;

    while (lptr != NULL && strcmp(lptr->sprache, w2))
      lptr = lptr->next;

    if (lptr == NULL && !strcmp(w2, "G"))
      lptr = root;

    if (lptr != NULL) {
      rp = 0;
      found = false;
      if (lptr->lct[nr - 1].p != NULL) {
	get_line(lptr->lct[nr - 1].p, &rp, lptr->lct[nr - 1].s, s);
	found = true;
      }
    }

    if (!found) {
      if (!strcmp(w2, "G")) {
	strcpy(s, "error: incorrect language definitions");
	found = true;
      } else
	strcpy(w2, "G");
    }

  } while (!found);

}


/* Loescht geladene Sprachen	*/

void free_languages(languagetyp **root)
{
  languagetyp *hptr, *hp2;

  hptr = *root;
  while (hptr != NULL) {
    if (hptr->puffer != NULL)
      free(hptr->puffer);
    hp2 = hptr;
    hptr = hptr->next;
    free(hp2);
  }
  *root = NULL;
}


/* Ordnet ein Call einer Sprachanpassung zu */

void user_language(char **root, long *size, char *fname, char *call_, char *lan)
{
  long	lz;
  char	call[256], zl[256], w[256];

  strcpyupper(call, call_);
  strcpy(lan, "G");
  if (*root == NULL) {
    sfbread(true, fname, root, size);
  }
  if (*root == NULL) {
    return;
  }
  lz = 0;
  while (lz < *size) {
    get_line(*root, &lz, *size, zl);
    get_word(zl, w);
    upper(w);
    if (*w && strstr(call, w) == call) {
      get_word(zl, lan);
      upper(lan);
      return;
    }
  }
}


void load_languages(languagetyp **root, char *pfad)
{
  short x;
  long sz, l;
  char *p1;
  languagetyp *hptr, *lptr;
  short result;
  DTA dirinfo;
  char hs[256], w[256];
  char sprache[256];
  char pfad2[256];
  char extension[256];
  char STR1[256];

  free_languages(root);
  lptr = NULL;
  strcpy(pfad2, pfad);
  get_path(pfad2);
  result = sffirst(pfad, 0, &dirinfo);
  while (result == 0) {
    strcpy(sprache, dirinfo.d_fname);
    get_ext(sprache, extension);
    del_ext(sprache);
    hptr = malloc(sizeof(languagetyp));
    if (*root == NULL)
      *root = hptr;
    hptr->next = NULL;
    if (lptr != NULL)
      lptr->next = hptr;
    sprintf(STR1, "%s%s.%s", pfad2, sprache, extension);
    sfbread(true, STR1, &hptr->puffer, &hptr->psize);
    if (hptr->puffer != NULL) {
      cut(sprache, 3);   /*da der user auch nicht mehr angeben kann...*/
      strcpyupper(hptr->sprache, sprache);
      lptr = hptr;

      for (x = 0; x < MAXLANGUAGELINEDEFS; x++)
	hptr->lct[x].p = NULL;

      sz = 0;
      while (sz < hptr->psize) {
	p1 = (char *)(&hptr->puffer[sz]);
	l = sz;
	get_line(hptr->puffer, &sz, hptr->psize, hs);
	get_word(hs, w);
	x = atoi(w);
	if (x > 0 && x <= MAXLANGUAGELINEDEFS && *hs != '\0') {
	  l = hptr->psize - l - strlen(w) - 1;
	  if (l > 0) {
	    hptr->lct[x - 1].p = (char *)(&p1[strlen(w) + 1]);
	    hptr->lct[x - 1].s = l;
	  }
	}
      }

    } else
      free(hptr);
    result = sfnext(&dirinfo);
  }
}


void stripcr(char *fn)
{
  short k, h;
  char l[256], fn2[256];
  boolean bin, ok;
  char *p;
  long ps, err, fp1, fp2;

  ok = false;

  strcpy(fn2, fn);
  validate(fn2);

  k = sfopen(fn, FO_READ);
  if (k < minhandle)
    return;
  h = sfcreate(fn2, FC_FILE);
  if (h >= minhandle) {
    bin = false;
    ok = true;

    while (!bin && file2str(k, l)) {
      bin = (l[0] == '#' && strstr(l, "#BIN#") == l);
      str2file(&h, l, true);
    }

    if (bin) {
      fp1 = sfseek(0, k, SFSEEKCUR);
      fp2 = sfseek(0, k, SFSEEKEND);
      ps = fp2 - fp1 + 1;
      sfseek(fp1, k, SFSEEKSET);
      p = malloc(ps);
      if (p != NULL) {
	err = sfread(k, ps, p);
	sfwrite(h, err, p);
	free(p);
      } else
	ok = false;
    }

    sfclose(&h);
  }

  sfclose(&k);

  if (ok) {
    sfdelfile(fn);
    sfrename(fn2, fn);
  }

}


boolean get_fileline(char *fn, short line, char *l)
{
  short k, c;

  *l = '\0';
  k = sfopen(fn, FO_READ);
  if (k < minhandle)
    return false;
  c = 1;
  while (file2str(k, l) && c < line)
    c++;
  sfclose(&k);
  return (c == line);
}


boolean get_keyline(char *fn, char *key, boolean casesense, char *l)
{
  boolean Result;
  short k;
  boolean ok;
  char l1[256], l2[256], k1[256];

  Result = false;
  if (*key == '\0') return Result;
  *l = '\0';
  k = sfopen(fn, FO_READ);
  if (k < minhandle)
    return Result;
  if (casesense) {
    while (file2str(k, l) && strstr(l, key) != l) ;
    Result = (strstr(l, key) == l);
  } else {
    strcpyupper(k1, key);
    ok = false;
    while (!ok && file2str(k, l)) {
      strcpyupper(l1, l);
      get_word(l1, l2);
      ok = (strcmp(k1, l2) == 0);
    }
    Result = ok;

  }
  sfclose(&k);
  return Result;
}


boolean replace_keyline(char *fn, char *key, boolean casesense, char *e)
{
  boolean Result;
  short k, h;
  boolean ok;
  char l[256], l1[256], l2[256], k1[256];
  char fn2[256];

  Result = false;
  if (*key == '\0') return Result;

  strcpy(fn2, fn);
  validate(fn2);

  k = sfopen(fn, FO_READ);
  if (k < minhandle)
    return Result;
  h = sfcreate(fn2, FC_FILE);
  if (h >= minhandle) {
    if (casesense) {
      while (file2str(k, l) && strstr(l, key) != l)
	str2file(&h, l, true);
      Result = (strstr(l, key) == l);
      str2file(&h, e, true);
      while (file2str(k, l))
	str2file(&h, l, true);
    } else {
      strcpyupper(k1, key);
      ok = false;
      while (!ok && file2str(k, l)) {
	strcpyupper(l1, l);
	get_word(l1, l2);
	ok = (strcmp(k1, l2) == 0);
	if (!ok)
	  str2file(&h, l, true);
      }
      Result = ok;
      str2file(&h, e, true);
      while (file2str(k, l))
	str2file(&h, l, true);
    }
    sfclose(&h);
  }
  sfclose(&k);
  if (exist(fn2)) {
    sfdelfile(fn);
    sfrename(fn2, fn);
  }
  return Result;
}


/* -------------------------------------------------------------------------------------------- */
/* MD2 / MD5 calls */


void calc_MD5_pw(char *MD5prompt, char *MD5pw, char *MD5result)
{
  MD5_CTX context;
  MD2barr16 digest;
  short i, n, len;
  char c1, c2;
  char buff[256];

  *MD5result = '\0';
  len = strlen(MD5prompt) + strlen(MD5pw);
  if (len > 255)
    return;
  strcpy(buff, MD5prompt);
  strcat(buff, MD5pw);

  MD5Init(&context);

  i = 0;

  while (i < len) {
    if (len - i > 16)
      n = 16;
    else
      n = len - i;
    MD5Update(&context, (char *)(&buff[i]), n);
    i += 16;
  }
  MD5Final(digest, &context);

  for (i = 0; i <= 15; i++) {
    int2hchar(digest[i], &c1, &c2);
    sprintf(MD5result + strlen(MD5result), "%c%c", c1, c2);
  }
  lower(MD5result);
}

void calc_MD2_pw(char *MD2prompt, char *MD2pw, char *MD2result)
{
  MD2_CTX context;
  MD2barr16 digest;
  short i, n, len;
  char c1, c2;
  char buff[256];

  *MD2result = '\0';
  len = strlen(MD2prompt) + strlen(MD2pw);
  if (len > 255)
    return;
  strcpy(buff, MD2prompt);
  strcat(buff, MD2pw);

  MD2Init(&context);

  i = 0;

  while (i < len) {
    if (len - i > 16)
      n = 16;
    else
      n = len - i;
    MD2Update(&context, (char *)(&buff[i]), n);
    i += 16;
  }
  MD2Final(digest, &context);

  for (i = 0; i <= 15; i++) {
    int2hchar(digest[i], &c1, &c2);
    sprintf(MD2result + strlen(MD2result), "%c%c", c1, c2);
  }
  lower(MD2result);
}

void calc_MD_prompt(boolean only_numbers, char *MDprompt)
{
  short x;
  char c;

  MDprompt[0] = '\0';
  for (x = 1; x <= 10; x++) {
    if (only_numbers) {
      do {
        c = dp_randomize(48, 57);
      } while (!isdigit(c));
    } else {
      do {
        c = dp_randomize(48, 122);
      } while (!isalnum(c));
    }
    sprintf(MDprompt + strlen(MDprompt), "%c", c);
  }
}


/* -------------------------------------------------------------------------------------------- */


/* Ist glaube ich urspruenglich von DL5FBD

(***********************************************************)
(* Procedure Entfernung_Richtung                           *)
(* Die Prozedur dient zur Berechnung von Entfernung und    *)
(* Richtung bei gegebenen geografischen Koordinaten im     *)
(* Gradmass.                                               *)
(* Ergebnis sind Entfernung in Kilometern und Richtung in  *)
(* Grad von QTH1 nach QTH2.                                *)
(* O1,N1 Oestliche Laenge,Noerdliche Breite von QTH1       *)
(* O2,N2 Oestliche Laenge,Noerdliche Breite von QTH2       *)
(***********************************************************)

*/
#ifndef PI
#define PI 3.141592654 /*Kreiskonstante PI    */
#endif

/* Funktion GSIN                                           */
/* Berechnung des Sinus zu einem gegebenen Gradwinkel      */

static double gsin(double winkel)
{
  return sin(winkel*PI/180);
}


/* Funktion GCOS                                           */
/* Berechnung des Cosinus zu einem gegebenen Gradwinkel    */

static double gcos(double winkel)
{
  return cos(winkel*PI/180);
}


/* Funktion ARCGCOS                                        */
/* Berechnung des Gradwinkels zum gegebenen Cosinuswert    */

static double arcgcos(double cosinus)
{
  double arcbog;            /*Hilfsvariable vor Gradumrechnung*/
  double sqrh;

  if (cosinus >= 1.0) return 0;          /*Sonderfall   0 Grad*/
  else if (cosinus <= -1.0) return 180;  /*Sonderfall 180 Grad*/

  sqrh = (1-pow(cosinus, 2));
  if (sqrh < 0.0) sqrh = 0.0;
  arcbog = PI/2-atan(cosinus/(sqrt(sqrh)));
  /*Umrechnung vom Bogenmaß in Grad*/
  return arcbog*180/PI;
}


void loc_dist(double o1, double n1, double o2, double n2,
              double *entfernung, double *richtung, double *gegenrichtung)
{
  double ew,rv,zs;  /*EW Entfernungswinkel */
                    /*RV vorlaeufige Richtg*/

  if ((o1 == o2) && (n1 == n2)) {
    *entfernung = 0;
    *richtung = 0;
    *gegenrichtung = 0;
    
  } else {
  
    /* Entfernungsberechnung */

    ew = arcgcos(gsin(n1)*gsin(n2)+gcos(n1)*gcos(n2)*gcos(o2-o1));
    *entfernung = 40009/360*ew;

    /* Richtungsberechnung */

    zs  = gcos(n1)*gsin(ew);
    
    if (zs != 0)
      rv = arcgcos((gsin(n2)-gsin(n1)*gcos(ew))/zs);
    else
      rv = 0;
      
    if (gsin(o2-o1) >= 0) *richtung = rv;
    if (gsin(o2-o1) < 0) *richtung = 360-rv;

    zs = gcos(n2)*gsin(ew);
    
    if (zs != 0)
      rv = arcgcos((gsin(n1)-gsin(n2)*gcos(ew))/zs);
    else
      rv = 0;
      
    if (gsin(o1-o2) >= 0) *gegenrichtung = rv;
    else *gegenrichtung = 360-rv;
  }
}

/*********** Ende Entfernung_Richtung ************/



/* Wandeln des WW-Locators in Laengen- und Breitengrad mit Formatueberpruefung */
boolean calc_qth(char *loc_, double *l, double *b)
{
  char loc[256];
  
  if (strlen(loc_) == 6) {      /* zuerst wird die Locatorangabe geprueft */
    strcpyupper(loc, loc_);
    if ((loc[0] <= 'R' && loc[0] >= 'A') && (loc[1] <= 'R' && loc[1] >= 'A')) {
      if ((loc[2] <= '9' && loc[2] >= '0') && (loc[3] <= '9' && loc[3] >= '0')) {
        if ((loc[4] <= 'X' && loc[4] >= 'A') && (loc[5] <= 'X' && loc[5] >= 'A')) {
          *b = ((loc[1] - 65)*10 - 90.0) + (loc[3] - 48.0)
                + ((loc[5] - 65)/24.0 + 1.0/48.0);
          *l = ((loc[0] - 65)*20 - 180.0) + ((loc[2] - 48.0)*2.0)
                + ((loc[4] - 65)/12.0 + 1.0/24.0);
          return true;
        }
      }
    }
  }
  return false;
}


boolean get_wwloc(char *inp, char *outp)
{
  double  lon, lat;
  char	  *p, *s, hs[256];
  
  nstrcpy(hs, inp, 255);
  upper(hs);
  p = hs;
  while (*p) {
    if (!isupper(*p) && !isdigit(*p)) *p = '\0';
    p++;
  }
  s = hs;
  while (s < p) {
    if (!*s) s++;
    else {
      if (calc_qth(s, &lon, &lat)) {
      	strcpy(outp, s);
	return true;
      } else while ((*s++));
    }
  }
  *outp = '\0';
  return false;
}


static void substitute(char *call, char *subst)
{
  short k;
  char hs1[256];
  boolean found;
  long subsize;
  char *buff;
  long lz;
  
  strcpy(subst, call);
  sprintf(hs1, "%sprefix%csub", boxsysdir, extsep);
  subsize = sfsize(hs1);
  if (subsize > 5) {
    buff = malloc(subsize);
    if (buff != NULL) {
      k = sfopen(hs1, FO_READ);
      if (k >= minhandle) {
        subsize = sfread(k, subsize, buff);
        sfclose(&k);
        found = false;
        lz = 0;
        while ((lz < subsize) && (!found)) {
          get_line(buff, &lz, subsize, hs1);
          if (*hs1 && strstr(call, hs1) == call) {
            found = true;
            strdelete(call, 1, strlen(hs1));
            *hs1 = '\0';
            if (lz < subsize)
              get_line(buff, &lz, subsize, hs1);
            if (strlen(hs1) + strlen(call) <= 8) {
               strcpy(subst, hs1);
               strcat(subst, call);
            } else
              strcpy(subst, hs1);
          } else if (lz < subsize) {
            get_line(buff, &lz, subsize, hs1);
          }
        }
      }
      free(buff);
    }
  }
}


boolean prefix(char *call_, char *subst, char *name, double *lon, double *lat,
		short *waz, short *itu, char *continent)
{
  char hs[256], call[256];
  boolean found, Result;
  short k;
  long infsize, lz;
  char *buff;
  
  Result = false;
  strcpyupper(call, call_);
  cut(call, LEN_CALL);
  substitute(call, subst);
  sprintf(hs, "%sprefix%cinf", boxsysdir, extsep);
  infsize = sfsize(hs);
  if (infsize > 5) {
    buff = malloc(infsize);
    if (buff != NULL) {
      k = sfopen(hs, FO_READ);
      if (k >= minhandle) {
        infsize = sfread(k, infsize, buff);
        sfclose(&k);
        found = false;
        lz = 0;
        while ((lz < infsize) && (!found)) {
          get_line(buff, &lz, infsize, hs);
          if (*hs && strstr(subst, hs) == subst) {
            found = true;
            Result = true;
            if (lz < infsize)
              get_line(buff, &lz, infsize, name);
            gkdeutsch(name);
            if (lz < infsize)
              get_line(buff, &lz, infsize, hs);
            *waz = ((short)(hs[0])-(short)('0'))*10 + ((short)(hs[1])-(short)('0'));
            *itu = ((short)(hs[2])-(short)('0'))*10 + ((short)(hs[3])-(short)('0'));
            if (*waz < 1 || *waz > 255)
              *waz = 0;
            if (*itu < 1 || *itu > 255)
              *itu = 0;
            strsub(continent, hs, 5, 2);
            if (lz < infsize)
              get_line(buff, &lz, infsize, hs);
            *lon = atof(hs);
            if (lz < infsize)
              get_line(buff, &lz, infsize, hs);
            *lat = atof(hs);
          } else {
            for (k = 1; k <= 4; k++) {
              if (lz < infsize)
                get_line(buff, &lz, infsize, hs);
            }
          }
        }
      }
      free(buff);
    }
  }
  return Result;
}

/* small digiinfo-support */

static void mystrtok(char **inp1, char *outp, char delim)
{
  char *p, *inp;
  
  inp = *inp1;
  p = outp;
  while (*inp) {
    if (*inp == delim) {
      inp++;
      break;
    }
    *p++ = *inp++;
  }
  *p = '\0';
  *inp1 = inp;
  
  if (*outp) del_blanks(outp);
}

boolean get_digimap_data(boolean bbs, char *call_, char *sysop, char *mbsysop, char *qthloc,
			 char *qthname, char *mbsystem, char *digisystem, char *remarks,
			 char *updatetime)
{
  short k, ret, state;
  long size, pos, interval;
  boolean is_mb;
  char hs[256], call[256], filename[256], line[256];
  char tcall[80], tstate[80], ttyp[80], tsoft[80], thard[80], tfreq[80],
       tbaud[80], tsysop[80], tloc[80], tqth[80], trem[80], ti[80], tupdate[80];
  char *s, *p;
  
  *sysop = '\0';
  *qthloc = '\0';
  *qthname = '\0';
  *mbsysop = '\0';
  *mbsystem = '\0';
  *digisystem = '\0';
  *remarks = '\0';
  *updatetime = '\0';
  
  sprintf(filename, "%sdigimap%cstn", boxsysdir, extsep);
  size = sfsize(filename);
  if (size < 200)
    return false;
  
  interval = size / 2;
  pos = interval;
  
  k = sfopen(filename, FO_READ);
  if (k < minhandle)
    return false;
  
  del_callextender(call_, call);
  upper(call);
    
  while (interval > 20 && pos >= 0 && pos < size) {
  
    if (sfseek(pos, k, SFSEEKSET) != pos) {
      sfclose(&k);
      return false;
    }
    
    interval = interval / 2;

    file2str(k, hs);
    file2str(k, hs);
    
    s = strtok(hs, "-, ");
    if (s == NULL)
    	ret = 1;
    else
    	ret = strcmp(call, s);
    if (ret == 0) {

      pos = pos - 1500;
      if (pos < 0) pos = 0;
      if (sfseek(pos, k, SFSEEKSET) != pos) {
        sfclose(&k);
        return false;
      }
      file2str(k, hs);
      state = 0;
      while (state < 15) {
        file2str(k, line);
        strcpy(hs, line);
        s = strtok(hs, "-, ");
        if (s != NULL && !strcmp(call, s)) {
          state++;

      	  p = line;
          mystrtok(&p, tcall, ',');
          mystrtok(&p, ti, ',');
          mystrtok(&p, tstate, ',');
          mystrtok(&p, tupdate, ',');
          mystrtok(&p, ttyp, ',');
          mystrtok(&p, tsoft, ',');
          mystrtok(&p, thard, ',');
          mystrtok(&p, tfreq, ',');
          mystrtok(&p, tbaud, ',');
          mystrtok(&p, tsysop, ',');
          mystrtok(&p, tloc, ',');
          mystrtok(&p, tqth, ',');
          mystrtok(&p, trem, ',');
                    
          is_mb = false;
          
          if (*updatetime == '\0' || strcmp(tupdate, updatetime) > 0)
            strcpy(updatetime, tupdate);

          if (!strcmp("DI", ttyp)) {
            if (*digisystem == '\0')
             strcpy(digisystem, tsoft);
            else if (strstr(digisystem, tsoft) == NULL) {
              strcat(digisystem, ",");
              strcat(digisystem, tsoft);
            }
          } else if (!strcmp("MB", ttyp)) {
            is_mb = true;
            if (*mbsystem == '\0')
             strcpy(mbsystem, tsoft);
            else if (strstr(mbsystem, tsoft) == NULL) {
              strcat(mbsystem, ",");
              strcat(mbsystem, tsoft);
            }
          }          

          if (*tsysop != '\0') {
            if (is_mb)
              strcpy(mbsysop, tsysop);
            if (*sysop == '\0')
              strcpy(sysop, tsysop);
            else if (strstr(sysop, tsysop) == NULL) {
              strcat(sysop, ", ");
              strcat(sysop, tsysop);
            }
          }
          
          if (*tloc != '\0' && (*qthloc == '\0' || (bbs && is_mb)))
            strcpy(qthloc, tloc);

          if (*tqth != '\0' && (*qthname == '\0' || (bbs && is_mb)))
            strcpy(qthname, tqth);
            
          if (*trem != '\0') {
            if (*remarks == '\0')
              strcpy(remarks, trem);
            else if (strlen(remarks) + strlen(trem) < 78) {
              strcat(remarks, " ");
              strcat(remarks, trem);
            }
          }
          
        } else if (state > 0)
          state = 100;
      }
    
      sfclose(&k);
      return true;

    } else if (ret < 0)
      pos -= interval;
    else 
      pos += interval;
    
  }
  
  sfclose(&k);
  return false;
}

static void sort_dm_file(char *name)
{
  gnusort(name, "-u", "-k 1,11");
}

static short prepare_digimap_update(char *updfile, char *addfile, char *delfile)
{
  short   k, l, ct;
  short   addf, delf, updf;
  char	  c, what;
  char	  hs[256], hs2[256];

  updf	= sfopen(updfile, FO_READ);
  addf	= sfcreate(addfile, FC_FILE);
  delf	= sfcreate(delfile, FC_FILE);
  if (updf < minhandle || addf < minhandle || delf < minhandle) {
    sfclose(&updf);
    sfclosedel(&addf);
    sfclosedel(&delf);
    return -1;
  }
  
  while (file2str(updf, hs)) {
    del_leadblanks(hs);
    if (!*hs || *hs == '#') continue;
    l 	= strlen(hs);
    if (l < 6) continue;
    if (hs[2] != ':') continue;
    if (upcase_(hs[1]) != 'S') continue; /* only stations, no links */
    what  = upcase_(hs[0]);
    if (what != 'A' && what != 'D' && what != 'M') continue; /* only add, delete, modify allowed */
    del_lastblanks(hs);
    
    /* change to uppercase until 11th comma */
    ct	= 0;
    k 	= 0;
    while ((c = hs[k+3])) {
      if (c == ',') ct++;
      else if (ct < 11) c = upcase_(c);
      hs2[k++] = c;
    } 
    hs2[k] = '\0';
    
    /* write result in add- or delfile */
    if (*hs2) {
      switch (what) {
      	case 'A':     	
	case 'M': str2file(&addf, hs2, true);
	      	  break;
	case 'D': str2file(&delf, hs2, true);
	      	  break;
      	default : break;
      }
    }
  }
  
  sfclose(&updf);
  sfclose(&addf);
  sfclose(&delf);
  sort_dm_file(addfile);
  sort_dm_file(delfile);
  return 0;
}

static boolean get_next_dm(short handle, char *out, char *outs, char *lasts)
{
  boolean meof;
  
  *outs = '\0';
  *out	= '\0';
  if (handle < minhandle) return false;
  do {
    meof = file2str(handle, out);
    nstrcpy(outs, out, 11);
  } while (!strcmp(outs, lasts) && meof);
  strcpy(lasts, outs);
  return meof;
}

static void put_dm(short *handle, char *out)
{
  if (handle < minhandle) return;
  str2file(handle, out, true);
  *out	= '\0';
}

/* merging an update file into base (system/digimap.stn)  */

short merge_digimap_data(char *base, char *updfile)
{
  boolean infok, addfok, delfok, delok;
  short   addf, delf, origf, outf, comp;
  char	  orig[256], add[256], del[256], adds[256], dels[256], origs[256];
  char	  lastorigs[256], lastadds[256], lastdels[256];
  pathstr newname, addfile, delfile;
  
  if (!exist(base)) return -1;
  if (!exist(updfile)) return -2;

  strcpy(addfile, base);
  strcat(addfile, ".add");
  strcpy(delfile, base);
  strcat(delfile, ".del");
  strcpy(newname, base);
  strcat(newname, ".new");
  if (prepare_digimap_update(updfile, addfile, delfile) < 0) return -3;

  sort_dm_file(base);

  addf	= sfopen(addfile, FO_READ);
  delf	= sfopen(delfile, FO_RW);
  origf = sfopen(base, FO_RW);
  outf	= sfcreate(newname, FC_FILE_RALL);
  if (addf < minhandle || delf < minhandle || origf < minhandle || outf < minhandle) {
    sfclosedel(&addf);
    sfclosedel(&delf);
    sfclose(&origf);
    sfclosedel(&outf);
    return -4;
  }

  *add = '\0';
  *del = '\0';
  *orig= '\0';
  *lastadds = '\0';
  *lastdels = '\0';
  *lastorigs = '\0';
  infok= true;
  addfok= true;  
  delfok = get_next_dm(delf, del, dels, lastdels);
  do {
  
    do {
      if (infok && !*orig) infok = get_next_dm(origf, orig, origs, lastorigs);
      while (delfok && strncmp(dels, origs, strlen(dels)) < 0)
      	delfok = get_next_dm(delf, del, dels, lastdels);
      if (delfok && infok && *dels && *origs && (strstr(origs, dels) == origs)) {
      	delok = true;
	*orig = '\0';
	*lastorigs = '\0';
      } else delok = false;
    } while (delok);

    if (addfok && !*add) addfok = get_next_dm(addf, add, adds, lastadds);
    
    if (addfok) {
      comp = strcmp(adds, origs);
      if (comp <= 0) {
      	put_dm(&outf, add);
      	if (comp == 0) *orig = '\0';
      	continue;
      }
    }

    if (*orig) put_dm(&outf, orig);
      
  } while (infok);
  
  while (get_next_dm(addf, add, adds, lastadds)) put_dm(&outf, add);

  sfclosedel(&addf);
  sfclosedel(&delf);
  sfclosedel(&origf);
  sfclose(&outf);
  sfrename(newname, base);
  return 0;
}



/* Modified for use in DPBOX by Joachim Schurig, DL8HBS   */
/* code based on FUZZYSRCH.C of Reinhard Rapp, c't 4/1997 */
/* original header follows:                               */

/*    Unscharfe Suche im Information Retrieval               */
/*    (C) 1997 Reinhard Rapp                                 */
/*    ANSI C, getestet mit GNU C unter UNIX und MS-DOS,      */
/*    Borland C 5.0 unter Windows 95,                        */
/*    Metrowerks C (CW10) unter MacOS                        */

/* #include <stdio.h>
   #include <stdlib.h>
   #include <ctype.h>
   #include <string.h> */

#define  MaxParLen 1000   /* maximale Laenge eines Absatzes */

/* Grossbuchstaben in Kleinbuchstaben, Satzzeichen und div.  */
/* Sonderzeichen in Leerzeichen umwandeln, Laenge ermitteln  */
static long PrepareTheString(char* ConvStr, char* OriginStr)
{
  char*  TmpPtr;

  for (TmpPtr = ConvStr; *OriginStr; TmpPtr++, OriginStr++)
    {
      *TmpPtr = tolower(*OriginStr);
      if (*OriginStr < '0')  /* Satzzeichen neutralisieren   */
        *TmpPtr = ' ';
      else
        switch((unsigned char)*TmpPtr)
          {
            case 196: *TmpPtr = 228; break; /* ANSI-Umlaute  */
            case 214: *TmpPtr = 246; break;
            case 220: *TmpPtr = 252; break;
            case 142: *TmpPtr = 132; break; /* ASCII-Umlaute */
            case 153: *TmpPtr = 148; break;
            case 154: *TmpPtr = 129; break;
            case ':': *TmpPtr = ' '; break;
            case ';': *TmpPtr = ' '; break;
            case '<': *TmpPtr = ' '; break;
            case '>': *TmpPtr = ' '; break;
            case '=': *TmpPtr = ' '; break;
            case '?': *TmpPtr = ' '; break;
            case '[': *TmpPtr = ' '; break;
            case ']': *TmpPtr = ' '; break;
          }
    }
  *TmpPtr = '\0';
  return (TmpPtr - ConvStr);
}

/* sucht n-Gramme im Text und zaehlt die Treffer */
static int NGramMatch(char *TextPara, char *SearchStr,
               short SearchStrLen, short NGramLen, short *MaxMatch)
{
  char    NGram[8];
  short   NGramCount;
  short   i, Count;

  NGram[NGramLen] = '\0';
  NGramCount = SearchStrLen - NGramLen + 1;

/* Suchstring in n-Gramme zerlegen und diese im Text suchen */
  for(i = 0, Count = 0, *MaxMatch = 0; i < NGramCount; i++)
    {
      memcpy(NGram, &SearchStr[i], NGramLen);
      
      /* bei Wortzwischenraum weiterruecken */
      if (NGram[NGramLen - 2] == ' ' && NGram[0] != ' ')
          i += NGramLen - 3;
      else
        {
          *MaxMatch  += NGramLen;
          if(strstr(TextPara, NGram)) Count++;
        }
    }
  return Count * NGramLen;  /* gewichten nach n-Gramm-Laenge */
}

/* fuehrt die unscharfe Suche durch */
short FuzzyMatching(char *CheckStr, char* SearchStr,
		    short SearchStrLen, short NGram1Len, short NGram2Len)
{
  char    TextPara[MaxParLen] = " ";     /* vorn Leerzeichen */
  short   TextLen;
  short   MatchCount1, MatchCount2;
  short   MaxMatch1, MaxMatch2;

  /* Text absatzweise lesen und durchsuchen */
  TextLen = PrepareTheString(&TextPara[1], CheckStr);
  if (TextLen < MaxParLen - 2)
  {
      MatchCount1 = NGramMatch(TextPara, SearchStr,
                      SearchStrLen, NGram1Len, &MaxMatch1);
      MatchCount2 = NGramMatch(TextPara, SearchStr,
                      SearchStrLen, NGram2Len, &MaxMatch2);

      /* Trefferguete berechnen und Bestwert festhalten  */
      return calc_prozent((MatchCount1 + MatchCount2),
                          (MaxMatch1 + MaxMatch2));
  }

  return 0;

}

void PrepareFuzzySearch(char *SearchStr, char *NormalisedStr,
			short *SearchStrLen, short *NGram1Len, short *NGram2Len)
{
  /* n-Gramm-Laenge in Abhaengigkeit vom Suchstring festlegen*/
  *NormalisedStr = '\0';
  strcpy(NormalisedStr, " ");
  strcat(NormalisedStr, SearchStr);
  *SearchStrLen = PrepareTheString(NormalisedStr, NormalisedStr);
  *NGram1Len = 3;
  *NGram2Len = (*SearchStrLen < 7) ? 2 : 5;
}

#undef MaxParLen
/* End of FUZZYSRCH.C for DPBOX */
