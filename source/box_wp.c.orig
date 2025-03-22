/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#include <ctype.h>
#include "box_wp.h"
#include "tools.h"
#include "box_logs.h"
#include "box_sub.h"
#include "box_sf.h"
#include "sort.h"
#include "box_mem.h"
#include "box_rout.h"
#include "box.h"
#include "box_file.h"
#include "box_send.h"
#include "box_sys.h"



/* encapsulates a string in double quotes */

static void caps_string(char *s)
{
  char	hs[256];
  
  snprintf(hs, 255, "\"%s\"", s);
  strcpy(s, hs);
}

/* calculates a wprot checksum and inserts it in chars 0 and 1 of a given line */
static void set_wprot_checksum(char *p) {
  char	*s;
  short checksum;

  /* calc checksum of current line */
  checksum  = 0;
  s   	    = &p[3];
  while (*s) checksum = (checksum + *s++) & 0xFF;
  int2hchar(checksum, &p[0], &p[1]);
}


/* create wprot r line */
/* format: R bbs version timestamp hops quality */
static short set_wprot_r(char *p, wprottype *wpb) {
  
  if (wpb == NULL) return -1;
  if (wpb->which != 'R') return -1;
  if (!*wpb->call) return -1;
  if (!valid_wprot_timestamp(wpb->timestamp)) return -1;
  if (wpb->hops < 0) return -1;
  if (wpb->hops > MAXWPHOPS) return -1;
  snprintf(p, 999, "   R %s %ld %ld %d %ld",
    	   wpb->call, wpb->version, wpb->timestamp, wpb->hops+1, wpb->quality);
  set_wprot_checksum(p);
  return 0;
}

static short set_own_wprot_r(char *p) { /* set own route information */
  wprottype wpb;
  
  memset(&wpb, 0, sizeof(wprottype));
  wpb.which = 'R';
  wpb.timestamp = clock_.ixtime;
  /* take care we never create a route broadcast that looks like a phantom broadcast */
  if (is_phantom_timestamp(wpb.timestamp)) wpb.timestamp++;
  nstrcpy(wpb.call, Console_call, LEN_CALL);
  wpb.hops = 0;
  wpb.version = WPROT_VERSION;
  wpb.quality = 0;
  return set_wprot_r(p, &wpb);
}

/* fetch wprot r line */
/* format: R bbs version timestamp hops quality */
static short get_wprot_r(wprottype *wpb, char *p, char *actsender) {
  char	    hs[1000];
  char	    *s;
  
  debug0(5, -1, 227);

  if (strlen(p) > 999) return -1;
  
  memset(wpb, 0, sizeof(wprottype));
  
  nstrcpy(wpb->rxfrom, actsender, LEN_CALL);
  
  get_pquoted(&p, hs);  /* bbs-call */
  unhpath(hs, hs);
  if (!callsign(hs)) return -1;
  nstrcpy(wpb->call, hs, LEN_CALL);

  get_pquoted(&p, hs);  /* version */
  s = hs;
  while (*s) if (!isdigit(*s++)) return -1;
  wpb->version = atol(hs);
  if (!valid_wprot_version(wpb->version)) return -1;

  get_pquoted(&p, hs);  /* timestamp */
  s = hs;
  while (*s) if (!isdigit(*s++)) return -1;
  wpb->timestamp = atol(hs);
  if (!valid_wprot_timestamp(wpb->timestamp)) return -1;

  get_pquoted(&p, hs); /* hops */
  s = hs;
  while (*s) if (!isdigit(*s++)) return -1;
  wpb->hops = atoi(hs);
  if (wpb->hops < 1) return -1;
  if (wpb->hops > MAXWPHOPS) return -1;
  
  get_pquoted(&p, hs);  /* quality */
  s = hs;
  while (*s) if (!isdigit(*s++)) return -1;
  wpb->quality = atol(hs);

  get_pquoted(&p, hs); /* must be empty */
  if (*hs) return -1; 
  
  wpb->which = 'R'; /* mark as valid */
  return 0;
}

/* create wprot b line */
static short set_wprot_b(char *p, wprottype *wpb) {
  calltype  sysop;
  char	    proto[81];
  char	    hwadr[81];
  
  if (wpb == NULL) return -1;
  if (wpb->which != 'B') return -1;
  if (!*wpb->bbs) return -1;
  if (!valid_wprot_timestamp(wpb->timestamp)) return -1;
  if (wpb->hops < 0) return -1;
  if (wpb->hops > MAXWPHOPS) return -1;
  nstrcpy(sysop, wpb->sysop, LEN_CALL);
  if (!*sysop) strcpy(sysop, "?");
  nstrcpy(proto, wpb->proto, 80);
  if (!*proto) strcpy(proto, "?");
  nstrcpy(hwadr, wpb->hwadr, 80);
  if (!*hwadr) strcpy(hwadr, "?");
  snprintf(p, 255, "   B %s %ld %u %s %s %s %ld %d",
    	   wpb->bbs, wpb->version, wpb->status, sysop, proto, hwadr, wpb->timestamp, wpb->hops+1);
  set_wprot_checksum(p);
  return 0;
}

static short set_own_wprot_b(char *p) { /* set own bbs information */
  wprottype wpb;
  
  memset(&wpb, 0, sizeof(wprottype));
  wpb.which = 'B';
  wpb.timestamp = clock_.ixtime;
  strcpy(wpb.bbs, ownhiername);
  wpb.hops = 0;
  strcpy(wpb.sysop, "");
  if (allow_direct_sf)
    wpb.status = 1;
  else
    wpb.status = 0;
  strcpy(wpb.proto, "AX25");
  if (boxboxssid() == 0)
    strcpy(wpb.hwadr, Console_call);
  else
    sprintf(wpb.hwadr, "%s-%d", Console_call, boxboxssid());
  wpb.version = WPROT_VERSION;
  
  return set_wprot_b(p, &wpb);
}

/* fetch wprot b line */
static short get_wprot_b(wprottype *wpb, char *p, char *actsender) {
  char	    hs[256];
  char	    *s;
  
  debug0(5, -1, 227);

  if (strlen(p) > 255) return -1;
  
  memset(wpb, 0, sizeof(wprottype));
  
  nstrcpy(wpb->rxfrom, actsender, LEN_CALL);
  
  get_pquoted(&p, hs);  /* h-adr */
  nstrcpy(wpb->bbs, hs, LEN_MBX);
  unhpath(wpb->bbs, hs);
  if (!callsign(hs)) return -1;
  nstrcpy(wpb->call, hs, LEN_CALL);

  get_pquoted(&p, hs);  /* version */
  s = hs;
  while (*s) if (!isdigit(*s++)) return -1;
  wpb->version = atol(hs);
  if (!valid_wprot_version(wpb->version)) return -1;

  get_pquoted(&p, hs);  /* status */
  if (!strcmp(hs, "?")) wpb->status = 0;
  else {
    s = hs;
    while (*s) if (!isdigit(*s++)) return -1;
    wpb->status = atoi(hs);
  }
  
  get_pquoted(&p, hs);  /* sysop */
  if (!strcmp(hs, "?")) *wpb->sysop = '\0';
  else nstrcpy(wpb->sysop, hs, LEN_CALL);
  
  get_pquoted(&p, hs);  /* proto */
  if (!strcmp(hs, "?")) *wpb->proto = '\0';
  else nstrcpy(wpb->proto, hs, 80);
  
  get_pquoted(&p, hs);  /* hwadr */
  if (!strcmp(hs, "?")) *wpb->hwadr = '\0';
  else nstrcpy(wpb->hwadr, hs, 80);

  get_pquoted(&p, hs);  /* timestamp */
  s = hs;
  while (*s) if (!isdigit(*s++)) return -1;
  wpb->timestamp = atol(hs);
  if (!valid_wprot_timestamp(wpb->timestamp)) return -1;

  get_pquoted(&p, hs); /* hops */
  s = hs;
  while (*s) if (!isdigit(*s++)) return -1;
  wpb->hops = atoi(hs);
  if (wpb->hops < 1) return -1;
  if (wpb->hops > MAXWPHOPS) return -1;
  
  get_pword(&p, hs);  /* must be empty */
  if (*hs) return -1;

  wpb->which = 'B'; /* mark as valid */
  return 0;
}

/* create wprot m line */
static short set_wprot_m(char *p, wprottype *wpb) {
  bidtype   bid;
  calltype  origin;
  char	    name[83];
  char	    zip[23];
  char	    qth[83];
  
  if (wpb == NULL) return -1;
  if (wpb->which != 'M') return -1;
  if (!*wpb->call) return -1;
  if (!*wpb->bbs) return -1;
  if (!valid_wprot_timestamp(wpb->timestamp)) return -1;
  if (wpb->hops < 0) return -1;
  if (wpb->hops > MAXWPHOPS) return -1;
  nstrcpy(bid, wpb->bid, LEN_BID);
  if (!*bid) strcpy(bid, "?");
  nstrcpy(origin, wpb->origin, LEN_CALL);
  if (!*origin) strcpy(origin, "?");
  nstrcpy(name, wpb->name, 80);
  if (!*name) strcpy(name, "?");
  else if (count_words(name) > 1) caps_string(name);
  nstrcpy(zip, wpb->zip, 20);
  if (!*zip) strcpy(zip, "?");
  else if (count_words(zip) > 1) caps_string(zip);
  nstrcpy(qth, wpb->qth, 80);
  if (!*qth) strcpy(qth, "?");
  else if (count_words(qth) > 1) caps_string(qth);
  snprintf(p, 255, "   M %s %s %s %ld %s %d %s %s %s",
      	   wpb->call, wpb->bbs, bid, wpb->timestamp, origin, wpb->hops+1, name, zip, qth);
  set_wprot_checksum(p);
  return 0;
}

/* fetch wprot m line */
static short get_wprot_m(wprottype *wpb, char *p, char *actsender) {
  char	    hs[256];
  char	    *s;

  debug0(5, -1, 228);

  if (strlen(p) > 255) {
    debug(5, -1, 228, "line too long");
    return -1;
  }
  
  memset(wpb, 0, sizeof(wprottype));
  
  nstrcpy(wpb->rxfrom, actsender, LEN_CALL);
  
  get_pquoted(&p, hs); /* call */
  if (!callsign(hs)) {
    debug(5, -1, 228, "no callsign");
    return -1;
  }
  nstrcpy(wpb->call, hs, LEN_CALL);
  
  get_pquoted(&p, hs); /* bbs */
  nstrcpy(wpb->bbs, hs, LEN_MBX);
  del_comment(hs, '.');
  if (!callsign(hs)) {
    debug(5, -1, 228, "no bbs callsign");
    return -1;
  }
  
  get_pquoted(&p, hs); /* bid */
  if (!strcmp(hs, "?")) *wpb->bid = '\0';
  else nstrcpy(wpb->bid, hs, LEN_BID);

  get_pquoted(&p, hs); /* timestamp */
  s = hs;
  while (*s) if (!isdigit(*s++)) {
    debug(5, -1, 228, "invalid digit in timestamp");
    return -1;
  }
  wpb->timestamp = atol(hs);
  if (!valid_wprot_timestamp(wpb->timestamp)) {
    debug(5, -1, 228, "invalid timestamp");
    return -1;
  }

  get_pquoted(&p, hs); /* origin */
  if (!strcmp(hs, "?")) *hs = '\0';
  if (*hs && !callsign(hs)) {
    debug(5, -1, 228, "no origin callsign");
    return -1;
  }
  nstrcpy(wpb->origin, hs, LEN_CALL);
  
  get_pquoted(&p, hs); /* hops */
  s = hs;
  while (*s) if (!isdigit(*s++)) {
    debug(5, -1, 228, "invalid digit in hops");
    return -1;
  }
  wpb->hops = atoi(hs);
  if (wpb->hops < 1) {
    debug(5, -1, 228, "hops < 1");
    return -1;
  }
  if (wpb->hops > MAXWPHOPS) {
    debug(5, -1, 228, "hops > MAXWPHOPS");
    return -1;
  }

  get_pquoted(&p, hs); /* name */
  if (!strcmp(hs, "?")) *wpb->name = '\0';
  else nstrcpy(wpb->name, hs, 80);

  get_pquoted(&p, hs); /* zip */
  if (!strcmp(hs, "?")) *wpb->zip = '\0';
  else nstrcpy(wpb->zip, hs, 20);

  get_pquoted(&p, hs); /* qth */
  if (!strcmp(hs, "?")) *wpb->qth = '\0';
  else nstrcpy(wpb->qth, hs, 80);

  get_pword(&p, hs);  /* must be empty */
  if (*hs) {
    debug(5, -1, 228, "trailing garbage in line");
    return -1;
  }

  wpb->which = 'M'; /* mark as valid */

  return 0;
}

/* create wprot e line */
static short set_wprot_e(char *p, wprottype *wpb) {
  bidtype   bid;
  calltype  origin;
  char	    comment[83];

  if (wpb == NULL) return -1;
  if (wpb->which != 'E') return -1;
  if (!*wpb->bbs) return -1;
  if (!*wpb->erasebid) return -1;
  if (!*wpb->issuer) return -1;
  if (wpb->hops < 0) return -1;
  if (wpb->hops > MAXWPHOPS) return -1;
  nstrcpy(bid, wpb->bid, LEN_BID);
  if (!*bid) strcpy(bid, "?");
  nstrcpy(origin, wpb->origin, LEN_CALL);
  if (!*origin) strcpy(origin, "?");
  nstrcpy(comment, wpb->comment, 80);
  if (!*comment) strcpy(comment, "?");
  else if (count_words(comment) > 1) caps_string(comment);
  snprintf(p, 255, "   E %s %s %s %s %s %d %s",
      	   wpb->bbs, origin, bid, wpb->erasebid, wpb->issuer, wpb->hops+1, comment);
  set_wprot_checksum(p);
  return 0;
}

/* fetch wprot e line */
static short get_wprot_e(wprottype *wpb, char *p, char *actsender) {
  char	    hs[256];
  char	    *s;

  debug0(5, -1, 229);

  if (strlen(p) > 255) return -1;
  
  memset(wpb, 0, sizeof(wprottype));
  
  nstrcpy(wpb->rxfrom, actsender, LEN_CALL);
  
  get_pquoted(&p, hs); /* flood */
  if (!*hs) return -1;
  nstrcpy(wpb->bbs, hs, LEN_MBX);
  
  get_pquoted(&p, hs); /* origin */
  if (!callsign(hs)) return -1;
  nstrcpy(wpb->origin, hs, LEN_CALL);
  nstrcpy(wpb->call, hs, LEN_CALL);
  
  get_pquoted(&p, hs); /* bid */
  if (!strcmp(hs, "?")) *wpb->bid = '\0';
  else nstrcpy(wpb->bid, hs, LEN_BID);

  get_pquoted(&p, hs); /* erasebid */
  if (!strcmp(hs, "?")) return -1;
  else nstrcpy(wpb->erasebid, hs, LEN_BID);
  if (!*wpb->erasebid) return -1;

  get_pquoted(&p, hs); /* issuer */
  if (!strcmp(hs, "?")) *wpb->issuer = '\0';
  else {
    if (!callsign(hs)) return -1;
    nstrcpy(wpb->issuer, hs, LEN_CALL);
  }
  
  get_pquoted(&p, hs); /* hops */
  s = hs;
  while (*s) if (!isdigit(*s++)) return -1;
  wpb->hops = atoi(hs);
  if (wpb->hops < 1) return -1;
  if (wpb->hops > MAXWPHOPS) return -1;

  get_pquoted(&p, hs); /* comment */
  if (!strcmp(hs, "?")) *wpb->comment = '\0';
  else nstrcpy(wpb->comment, hs, 80);

  get_pword(&p, hs);  /* must be empty */
  if (*hs) return -1;

  wpb->which = 'E'; /* mark as valid */

  return 0;
}

static short set_wprot_v(char *p) {

  snprintf(p, 255, "   V %d", WPROT_VERSION);
  set_wprot_checksum(p);
  return 0;
}



/* ****************************** for statistics **************************** */

void cleanup_routing_stat(short days)
{
  short       	inh, outh;
  time_t      	mintime, maxtime;
  pathstr     	infilename, outfilename;
  routstattype	rss;
  
  snprintf(infilename, LEN_PATH, "%s%s%cbox", boxstatdir, WPRSTATNAME, extsep);
  inh  	= sfopen(infilename, FO_READ);
  if (inh < minhandle) return;

  snprintf(outfilename, LEN_PATH, "%s%cUPDATE", infilename, extsep);
  sfdelfile(outfilename);
  outh	= sfcreate(outfilename, FC_FILE);
  if (outh < minhandle) {
    sfclose(&inh);
    return;
  }
  
  mintime = days * SECPERDAY;
  if (mintime < SECPERDAY) mintime = SECPERDAY;
  mintime = clock_.ixtime - mintime;
  maxtime = clock_.ixtime + (8 * 60 * 60);
  
  while (sfread(inh, sizeof(routstattype), (char *)&rss) == sizeof(routstattype)) {
    if (rss.timestamp < mintime || rss.timestamp > maxtime) continue;
    if (sfwrite(outh, sizeof(routstattype), (char *)&rss) == sizeof(routstattype)) continue;
    break;
  }
  
  sfclose(&inh);
  sfclose(&outh);
  sfdelfile(infilename);
  sfrename(outfilename, infilename);
}

#define CHARTX 70
#define CHARTY 20

void show_routing_stat(short unr, char *call, char *options, rsoutputproc out)
{
  short       	action, x, y;
  boolean     	mtdefault;
  time_t      	lasttime, mintime, maxtime, xstep, xtime;
  short       	handle;
  unsigned long minqual, maxqual, ystep, cury, avgqual, avgct;
  routstattype	rss;
  pathstr	filename;
  char	      	hs[256], w[256], w2[256];
  unsigned long ymin[CHARTX], ymax[CHARTX];
  boolean     	isset[CHARTX];
  char       	viahops[CHARTX];
  calltype    	viacall[CHARTX];

  if (!callsign(call)) return;
  snprintf(filename, LEN_PATH, "%s%s%cbox", boxstatdir, WPRSTATNAME, extsep);
  handle      	= sfopen(filename, FO_READ);
  if (handle < minhandle) {
    out(unr, "Statistics not available");
    return;
  }

  mintime   	= 0;
  mtdefault   	= true;
  lasttime    	= 0;
  maxtime     	= 0;
  minqual     	= LONG_MAX;
  maxqual     	= 1;

  action      = 0;  /* show chart   */
  get_word(options, w);
  
  if (useq(w, "-A")) {
    action    = 1;  /* show ascii   */
    get_word(options, w);
  }

  if (zahl(w))
    mintime   	= clock_.ixtime - atol(w) * SECPERDAY;
  
  while (sfread(handle, sizeof(routstattype), (char *)&rss) == sizeof(routstattype)) {
    if (strcmp(rss.call, call)) continue;
    if (rss.timestamp <= lasttime || rss.timestamp < mintime) continue;
    if (mtdefault == true) {
      mtdefault = false;
      mintime 	= rss.timestamp; /* set to first valid timestamp known */
    }
    maxtime   	= rss.timestamp;
    
    switch (action) {
      case 0  : lasttime = rss.timestamp;
      	      	if (rss.quality < minqual) minqual = rss.quality;
      	      	if (rss.quality > maxqual) maxqual = rss.quality;
		break;
      case 1  : lasttime = rss.timestamp;
		ix2string4(rss.timestamp, w);
		snprintf(hs, 79, "%s %7ld %2d %s", w, rss.quality, rss.hops, rss.rxfrom);
		out(unr, hs);
      	      	break;
      default : break;
    }
  }

 if (action == 0 && !mtdefault && sfseek(0, handle, SFSEEKSET) == 0) {
    /* second pass for graphical display */
    /* at this moment we know mintime, maxtime, minqual and maxqual */
    
    /* prepare for chart */
    for (x = 0; x < CHARTX; x++) {
      ymin[x] 	  = maxqual;
      ymax[x] 	  = minqual;
      isset[x]	  = false;
      viahops[x]  = '>';
      *viacall[x] = '\0';
    }
    avgqual   	= 0;
    avgct     	= 0;
    xtime     	= mintime;
    xstep     	= (maxtime - mintime) / CHARTX;
    if (xstep < 1) xstep = 1;
    ystep     	= (maxqual - minqual) / (CHARTY-1);
    if (ystep < 1) ystep = 1;
    
    lasttime  	= 0;
    while (sfread(handle, sizeof(routstattype), (char *)&rss) == sizeof(routstattype)) {
      if (strcmp(rss.call, call)) continue;
      if (rss.timestamp <= lasttime || rss.timestamp < mintime || rss.timestamp > maxtime) continue;
      switch (action) {
      	case 0  : x   = (rss.timestamp - mintime) / xstep;
	      	  if (x >= CHARTX) x = CHARTX-1;
		  if (ymin[x] > rss.quality) ymin[x] = rss.quality;
		  if (ymax[x] < rss.quality) ymax[x] = rss.quality;
		  avgqual += rss.quality;
		  avgct++;
		  nstrcpy(viacall[x], rss.rxfrom, LEN_CALL);
		  if (rss.hops <= 9) viahops[x] = ((char)rss.hops) + '0';
		  else viahops[x] = '>';
		  if (viahops[x] == '0') viahops[x] = '1';
		  isset[x] = true;
      	      	  break;
      	default : break;
      }
    }

    /* now display the chart */
    for (x = 1; x < CHARTX; x++) {
      if (!isset[x]) {
      	viahops[x] = viahops[x-1];
      	ymin[x] = ymin[x-1];
      	ymax[x] = ymax[x-1];
      }
    }

    switch (action) {
      case 0    : if (avgct > 0) avgqual = avgqual / avgct;
		  ix2string4(mintime, w);
		  ix2string4(maxtime, w2);
		  snprintf(hs, 200, " %*s : %s - %s / %ldh %ldm avgqual %ld",
		      	   LEN_CALL, call, w, w2, xstep / (60 * 60), (xstep % (60 * 60)) / 60, avgqual);
		  out(unr, hs);
		  out(unr, "");
      	      	  for (y = CHARTY-1; y >= 0; y--) {
      	      	    cury = minqual + y*ystep;
      	      	    for (x = 0; x < CHARTX; x++) {
		      if (ymin[x] < cury+ystep && ymax[x] >= cury) {
		      	w[x] = viahops[x];
		      } else {
		      	if (avgqual < cury+ystep && avgqual >= cury) {
		      	  w[x] = '.';
			} else {
		      	  w[x] = ' ';
			}
		      }
		    }
		    w[CHARTX] = '\0';
		    snprintf(hs, 200, "%7ld +%s", cury, w);
		    del_lastblanks(hs);
		    out(unr, hs);
      	      	  }
		  for (x = 0; x < CHARTX; x++) {
		    if (isset[x])
		      w[x] = '+';
		    else
		      w[x] = '-';
		  }
		  w[CHARTX] = '\0';
		  snprintf(hs, 200, "        +%s", w);
		  del_lastblanks(hs);
		  out(unr, hs);
		  
		  for (y = 0; y < LEN_CALL; y++) {
		    for (x = 0; x < CHARTX; x++) {
		      if (isset[x] && y < strlen(viacall[x])) {
		      	w[x] = viacall[x][y];
		      } else {
		      	w[x] = ' ';
		      }
		    }
		    w[CHARTX] = '\0';
		    snprintf(hs, 200, "         %s", w);
		    del_lastblanks(hs);
		    out(unr, hs);
		  }
		  
		  break;
      default 	: break;
    }
  
  }
  
  sfclose(&handle);
  
  /* no data found */
  if (mtdefault) {
    get_btext(unr, 45, w);
    out(unr, w);
  }
}
#undef CHARTX
#undef CHARTY

void write_routes(char *call, char *rxfrom, time_t timestamp, unsigned long quality, short hops)
{
  short       	handle;
  routstattype	rss;
  pathstr     	w;
  
  if (!do_routing_stats) return;
  memset(&rss, 0, sizeof(routstattype));
  nstrcpy(rss.call, call, LEN_CALL);
  nstrcpy(rss.rxfrom, rxfrom, LEN_CALL);
  rss.timestamp = timestamp;
  rss.quality = quality;
  rss.hops = hops;
  snprintf(w, LEN_PATH, "%s%s%cbox", boxstatdir, WPRSTATNAME, extsep);
  handle = sfopen(w, FO_RW);
  if (handle < minhandle)
    handle = sfcreate(w, FC_FILE);
  if (handle < minhandle) return;
  sfseek(0, handle, SFSEEKEND);
  if (sfwrite(handle, sizeof(routstattype), (char *)&rss) != sizeof(routstattype))
    do_routing_stats = false;
  sfclose(&handle);
}



/* ********************************************************************************** */


static void create_em_message(wprottype *wpb)
/* this function creates an old fashioned E/M message for thebox */
{
  indexstruct 	header;
  char	      	hs[256], w[20];
  
  if (wpb == NULL) return;
  if (wpb->which != 'E' && wpb->which != 'M') return;
  
  memset(&header, 0, sizeof(indexstruct));
  header.dest[0]	= '\0';
  header.fwdct		= 0;
  header.pmode		= 0;
  header.size		= 0;
  header.packsize	= 0;
  header.lifetime	= 0;
  header.txlifetime	= 0;
  header.deleted	= false;
  header.txdate		= 0;
  header.rxqrg		= boxaktqrg();
  strcpy(header.rxfrom, wpb->rxfrom);
  header.level		= 1;
  header.fwdct		= 0;
  header.msgflags	= 0;
  header.msgtype	= 'B';
  strcpy(header.id, wpb->bid);
  strcpy(header.absender, wpb->call);
  
  if (wpb->which == 'E') {
    strcpy(header.verbreitung, wpb->bbs);
    strcpy(header.betreff, wpb->erasebid);
  } else { /* M messages */
    strcpy(header.verbreitung, e_m_verteiler);
    snprintf(hs, LEN_SUBJECT, "%s %ld", wpb->bbs, wpb->timestamp);
    strcpy(header.betreff, hs);
  }

  sprintf(w, "%c", wpb->which);
  strcpy(hs, "*");
  if (*header.rxfrom != '\0') {
    vermerke_sf(-1, false, w, header.rxfrom, hs, header, "");
  }

}

#define wpbhlen 2+LEN_CALL+1+10+1+LEN_CALL+1
#define wpbhlen1 2+LEN_CALL+1+10+1
void add_wprotline(wprottype *wpb, boolean meta)
/* if meta, this goes into a temp file for later processing */
{
  static long dummycounter = 0;
  static time_t last_ixtime = 0;
  time_t  cur_ixtime;
  char	  hs[1000], sl[1000];
  pathstr w;
  
  if (wpb == NULL)
    return;  

  if (wpb->which == '\0')
    return;
  
  if (!callsign(wpb->rxfrom))
    return;

  switch (wpb->which) {
    case 'M' : 	if (set_wprot_m(hs, wpb) < 0) return;
      	      	snprintf(sl, wpbhlen+1, "M/%-*s %10ld %-*s ", LEN_CALL, wpb->call, wpb->timestamp, LEN_CALL, wpb->rxfrom);
		nstrcat(sl, hs, 255);
		break;
    case 'E' :  if (set_wprot_e(hs, wpb) < 0) return;
      	      	if (dummycounter >= 9999) dummycounter = 0;
		dummycounter++;
      	      	snprintf(sl, wpbhlen+1, "E/%-*s %4ld %-*s ", LEN_BID, wpb->erasebid, dummycounter, LEN_CALL, wpb->rxfrom);
      	      	nstrcat(sl, hs, 255);
      	      	break;
    case 'B' :  if (set_wprot_b(hs, wpb) < 0) return;
      	      	snprintf(sl, wpbhlen+1, "B/%-*s %10ld %-*s ", LEN_CALL, wpb->call, wpb->timestamp, LEN_CALL, wpb->rxfrom);
      	      	nstrcat(sl, hs, 255);
      	      	break;
    case 'R' :  if (!do_wprot_routing && !meta) return;
      	      	if (set_wprot_r(hs, wpb) < 0) return;
      	      	/* we need a different timestamp for each R line */
      	      	cur_ixtime = clock_.ixtime;
      	      	if (cur_ixtime <= last_ixtime) cur_ixtime = last_ixtime + 1;
      	      	last_ixtime = cur_ixtime;
      	      	snprintf(sl, wpbhlen+1, "R/%-*s %10ld %-*s ", LEN_CALL, wpb->call, cur_ixtime, LEN_CALL, wpb->rxfrom);
      	      	nstrcat(sl, hs, 255);
      	      	break;
    default  :	return;
  }

  if (meta) {
    snprintf(w, LEN_PATH, "%swprotmsg%ctmp", boxstatdir, extsep);
  } else {
    if (wpb->which == 'M') {
      /* also create WP entries */
      snprintf(w, LEN_PATH, "%swpmess%cdat", boxstatdir, extsep);
      append(w, sl, true);
    }
    snprintf(w, LEN_PATH, "%swprotmsg%cdat", boxstatdir, extsep);
  }
  append(w, sl, true);

  if (meta) new_mybbs_data = true;
}


/* this is the background function that processes the formerly received infos */
void do_emt(long *seekp)
{
  long		tc;
  short		k;
  pathstr	fn;
  char		hs[1000], w1[1000];

  tc	= get_cpuusage();
  sprintf(fn, "%swprotmsg%ctmp", boxstatdir, extsep);
  k	= sfopen(fn, FO_READ);
  if (k >= minhandle) {
    if (sfseek(*seekp, k, SFSEEKSET) == *seekp) {
      do {
	if (file2lstr(k, hs, 999)) {
	  *seekp	= sfseek(0, k, SFSEEKCUR);
	  get_word(hs, w1); /* Typ/call || bid */
	  get_word(hs, w1); /* updatetime */
	  get_word(hs, w1); /* rxfrom */
	  if (w1[0] == '!')
	    *w1		= '\0';
	  process_wprotline(hs, w1, false); /* second run, now real processing */
	} else
	  *seekp	= 0;
      } while (*seekp != 0 && get_cpuusage() - tc < ttask);
    } else
      *seekp	= 0;
    sfclose(&k);
  } else
    *seekp	= 0;

  if (*seekp == 0) {
    sfdelfile(fn);
    new_mybbs_data = false;
  }
}

/* this is the input for WP and E/M messages, not for WPROT messages! */
/* M @ THEBOX < DL1AAA $223456TESTUU DB0ABC 950278955 */
/* E @ THEBOX < DB0GR $123456123456 ERASEBID */
void sf_rx_emt1(char *eingabe, char *actwpfilesender)
{
  wprottype   	wpb;
  char		hs[256], w[256];

  memset(&wpb, 0, sizeof(wprottype));
  nstrcpy(wpb.rxfrom, actwpfilesender, LEN_CALL);
  get_word(eingabe, w); /* rubrik */
  if (strlen(w) > 1 || (w[0] != 'E' && w[0] != 'M')) return;
  wpb.which = w[0];
  get_word(eingabe, w); /* @ */
  if (strcmp(w, "@")) return;
  get_word(eingabe, w); /* flood */
  if (wpb.which == 'E') nstrcpy(wpb.bbs, w, LEN_MBX);
  get_word(eingabe, w); /* < */
  if (strcmp(w, "<")) return;
  get_word(eingabe, w); /* sender */
  if (!callsign(w)) return;
  nstrcpy(wpb.call, w, LEN_CALL);
  if (wpb.which == 'E') nstrcpy(wpb.origin, w, LEN_CALL);
  get_word(eingabe, w); /* BID */
  if (w[0] != '$') return;
  strdelete(w, 1, 1);
  if (strlen(w) > LEN_BID) return;
  nstrcpy(wpb.bid, w, LEN_BID);
  if (!strcmp(wpb.bid, WPDUMMYBID)) strcpy(wpb.bid, "");
  get_word(eingabe, w);
  if (wpb.which == 'E') { /* erasebid */
    if (strlen(w) > LEN_BID) return;
    nstrcpy(wpb.erasebid, w, LEN_BID);
    nstrcpy(wpb.issuer, wpb.call, LEN_BID);
  } else { /* mybbs */
    if (strlen(w) > LEN_MBX) return;
    unhpath(w, hs);
    if (!callsign(hs)) return;
    nstrcpy(wpb.bbs, w, LEN_MBX);
    get_word(eingabe, w); /* updatetime */
    if (!zahl(w)) return;
    wpb.timestamp = atol(w);
    /* check if we also have zip, name, qth in that line */
    get_word(eingabe, w);
    if (*w) { /* yes */
      if (strcmp(w, "?")) nstrcpy(wpb.zip, w, 80);
      get_word(eingabe, w);
      if (strcmp(w, "?")) nstrcpy(wpb.name, w, 80);
      del_lastblanks(eingabe);
      nstrcpy(wpb.qth, eingabe, 80);
    }
  }
  wpb.hops = 0;
  wpb.version = WPROT_VERSION;

  /* now that we have a valid wprot struct, add it to the temp file for later processing */
  
  add_wprotline(&wpb, true);
}

void sf_rx_emt(short unr, char *eingabe)
{
  char	hs[256];

  if (boxrange(unr))
    strcpy(hs, user[unr]->call);
  else
    *hs = '\0';
    
  sf_rx_emt1(eingabe, hs);
}


/*
M/DL8HBS  950309342 DL8HBS 66 M DL8HBS DL8HBS.DB0SIF.#HES.DEU.EU FPSFDPDL8HBS 950309342 ? 1 Joachim ? ?
*/
void generate_wprot_files(void)
{
  short		inf, outf, x;
  long	      	lct;
  boolean	send_bc;
  sfdeftype   	*sfp;
  pathstr     	wpmname, wp2name, wptname;
  char		hs[1000], w[1000], lc[1000], ll[1000], ts[1000];

  lastwprotcreate = clock_.ixtime;
  
  snprintf(wpmname, LEN_PATH, "%swprotmsg%cdat", boxstatdir, extsep);
  snprintf(wp2name, LEN_PATH, "%swprot%cdat",    boxstatdir, extsep);
  snprintf(wptname, LEN_PATH, "%swprfor%cdat",   boxstatdir, extsep);
  sfdelfile(wp2name);

  if (exist(wpmname)) {
    sort_file(wpmname, false);

    inf	= sfopen(wpmname, FO_READ);
    outf	= sfcreate(wp2name, FC_FILE);

    set_wprot_v(hs);	      	/* set version information */
    snprintf(ll, 255, "%-*s %s", LEN_CALL, Console_call, hs);
    str2file(&outf, ll, true);
    set_own_wprot_b(hs);    	/* set own box information */
    snprintf(ll, 255, "%-*s %s", LEN_CALL, Console_call, hs);
    str2file(&outf, ll, true);

    *hs	= '\0';
    *w	= '\0';
    *ll     	= '\0';
    while (file2lstr(inf, hs, 999)) { /* eine Zeile aus wprotmsg.dat holen */
      strcpy(lc, w);	      	  /* = w der nächsten Zeile des letzten Durchlaufs */
      get_word(hs, w);      	  /* = Usercall für den Eintrag */
      get_word(hs, ts);     	  /* = timestamp (ueberlesen) */
      if (strcmp(w, lc) && *ll) str2file(&outf, ll, true);
      strcpy(ll, hs);       	  /* sichern */
    }
    if (*ll) str2file(&outf, ll, true);
    sfclose(&inf);
    sfclose(&outf);
  }

  if (exist(wp2name)) {
    sfp = sfdefs;
    while (sfp != NULL) {
      if (sfp->em == EM_WPROT) {
	lct 	= 0;
	strcpy(ts, sfp->call); /* boxcall */
      	upper(ts);
	send_bc  = send_full_routing_bc(ts);
	dp_watchdog(2, 4711);   /* Watchdog resetten         */
	inf	= sfopen(wp2name, FO_READ);
	outf	= sfcreate(wptname, FC_FILE);
	while (file2lstr(inf, hs, 999)) {
	  get_word(hs, w);
	  if (strcmp(w, ts)) { /* == if not received from this partner */
	    if (hs[3] == 'R' || hs[3] == 'B') {
	      /* don't send back to origin */
      	      for (x = 0; x < LEN_CALL; x++) w[x] = hs[x+5]; /* extract origin */
	      w[LEN_CALL] = '\0';
	      del_comment(w, '.');
	      del_comment(w, ' ');
	      /* if this neighbour does not actively sends route bc's, only send my route bc */
	      if (send_bc || hs[3] != 'R' || !strcmp(w, Console_call)) {
      	      	/* and in any case, don't send records back to origin */
		if (strcmp(w, ts)) { /* OK, is not the origin */
	      	  str2file(&outf, hs, true);
		  lct++;
		}
	      }
	    } else if (hs[3] == 'E') {
	      if (sfp->send_em) {
	      	/* don't send if distribution is outside area */
      	        for (x = 0; x < LEN_CALL; x++) w[x] = hs[x+5]; /* extract distribution */
		w[LEN_CALL] = '\0';
		del_comment(w, '.');
		del_comment(w, ' ');
		if (forward_ok(ts, "ABCD", w, "", "", false, false)) {
	      	  str2file(&outf, hs, true);
		  lct++;
		}
	      }
      	    } else {
	      if (hs[3] != 'M' || sfp->send_em) {
	      	str2file(&outf, hs, true);
	      	lct++;
	      }
	    }
	  }
	}
	sfclose(&outf);
	sfclose(&inf);
	if (lct > 2) { /* first two lines contain static header (V and own B line) */
	  sprintf(w, "%s", wptname);
	  send_sysmsg("W", ts, "WPROT Update", w, 1, 'P', 1);
	}
	sfdelfile(wptname);
      }
      sfp = sfp->next;
    }

  }
  sfdelfile(wpmname);
  sfdelfile(wp2name);
}



boolean in_wpservers(char *call)
{
  sfdeftype   	*sfp;

  sfp = find_sf_pointer(call);
  if (sfp == NULL) return false;
  return (sfp->em == EM_WP);
}

/*
950309342 DL8HBS 66 M DL8HBS DL8HBS.DB0SIF.#HES.DEU.EU FPSFDPDL8HBS 950309342 ? 1 Joachim ? ?
*/
static void add_wpmsg(char *ll, short outf)
{
  char	 call[256], rxfrom[256], ts[256], w1[256], r[256], zip[256], name[256], qth[256];
  
  if (!*ll) return;
  get_word(ll, ts); 	/* timestamp (unix-format) */
  get_word(ll, rxfrom); /* rxfrom */
  get_word(ll, w1);   	/* wprot checksum (not used) */
  get_word(ll, w1);   	/* wprot type */
  if (strcmp(w1, "M")) return;
  get_word(ll, call);
  get_quoted(ll, w1); 	/* MyBBS */
  add_hpath(w1);
  get_quoted(ll, ts);  	/* BID (not used) */
  get_quoted(ll, ts);	/* timestamp (again) */
  get_quoted(ll, name);	/* origin (not used) */
  get_quoted(ll, name);	/* hops (not used) */
  get_quoted(ll, name);	/* name */
  get_quoted(ll, zip); 	/* zip */
  get_quoted(ll, qth); 	/* qth */
  if (!*zip || count_words(zip) > 1) strcpy(zip, "?");
  if (count_words(name) > 1) {
    strcpy(r, name);
    get_word(r, name);
  }
  if (!*name) strcpy(name, "?");
  if (!*qth) strcpy(qth, "?");
  ixdatum2string(atol(ts), ts); /* umwandeln in Datumsstring */
  snprintf(r, 255, "%s On %c%c%c%c%c%c %s/U @ %s zip %s %s %s",
	      	rxfrom, ts[6], ts[7], ts[3], ts[4], ts[0], ts[1], call, w1, zip, name, qth);
  str2file(&outf, r, true); /* und Zeile in wpmsg.dat ablegen */
}

/*
M/DL8HBS  950309342 DL8HBS 66 M DL8HBS DL8HBS.DB0SIF.#HES.DEU.EU FPSFDPDL8HBS 950309342 ? 1 Joachim ? ?
*/
void generate_wp_files(void)
{
  short		inf, outf;
  boolean	ok;
  sfdeftype   	*sfp;
  pathstr     	wpmname, wp2name, wptname;
  char		hs[256], w[256], lc[256], ll[256], ts[256];

  lastwpcreate	= clock_.ixtime;

  snprintf(wpmname, LEN_PATH, "%swpmess%cdat", boxstatdir, extsep);
  snprintf(wp2name, LEN_PATH, "%swpmsg%cdat",  boxstatdir, extsep);
  snprintf(wptname, LEN_PATH, "%swpfor%cdat",  boxstatdir, extsep);
  sfdelfile(wp2name);

  if (exist(wpmname)) {
    sort_file(wpmname, false);
    inf	= sfopen(wpmname, FO_READ);
    outf	= sfcreate(wp2name, FC_FILE);
    *hs	= '\0';
    *w	= '\0';
    *ll     	= '\0';
    while (file2str(inf, hs)) { /* eine Zeile aus wpmess.dat holen */
      strcpy(lc, w);	      	  /* = w der nächsten Zeile des letzten Durchlaufs */
      get_word(hs, w);      	  /* = Usercall für den Eintrag */
      if (strcmp(w, lc)) add_wpmsg(ll, outf);
      strcpy(ll, hs);
    }
    add_wpmsg(ll, outf); 	  /* und noch die letzte Zeile aus wpmess.dat verarbeiten */
    sfclose(&inf);
    sfclose(&outf);
  }

  if (exist(wp2name)) {
    sfp = sfdefs;
    while (sfp != NULL) {
      if (sfp->send_em && sfp->em == EM_WP) {
	strcpy(ts, sfp->call); /* boxcall */
      	upper(ts);
	ok	= false;
	dp_watchdog(2, 4711);   /* Watchdog resetten         */
	inf	= sfopen(wp2name, FO_READ);
	outf	= sfcreate(wptname, FC_FILE);
	while (file2str(inf, hs)) {
	  get_word(hs, w);
	  if (strcmp(w, ts)) {
	    str2file(&outf, hs, true);
	    ok = true;
	  }
	}
	sfclose(&outf);
	sfclose(&inf);
	if (ok) {
	  sprintf(w, "%s", wptname);
	  send_sysmsg("WP", ts, "WP Update", w, 1, 'P', 1);
	}
	sfdelfile(wptname);
      }
      sfp = sfp->next;
    }
  }
  sfdelfile(wpmname);
  sfdelfile(wp2name);
}




/* this function checks and analyzes the sent wprot line. */
/* when called, we are sure the first two chars contain   */
/* bare hex chars, followed by a space.	      	      	  */
/* return values:     	      	      	      	      	  */
/*               0 OK 	      	      	      	      	  */
/*              -1 invalid line       	      	      	  */
/*              -2 invalid protocol type      	      	  */

short process_wprotline(char *hs, char *actsender, boolean meta) {
/* meta means that all data is written into a temp file for later processing. */
/* in fact, for most message lines, this function is called twice. This helps */
/* limiting process time, as the database operations are performance critical */
  short       	checksum;
  long	      	version;
  time_t      	timestamp;
  unsigned long qual;
  wprottype   	wpb;
  char	      	*p, w[1000];
  
  /* char 5 must be a space in this protocol version */
  if (hs[4] != ' ') {
    debug(2, -1, 226, "invalid wprot format");
    return -1;
  }
  p   	    = &hs[3];
  /* calc checksum of current line */
  checksum  = 0;
  while (*p) checksum = (checksum + *p++) & 0xFF;
  /* compare checksum with sent checksum */
  hs[2] = '\0';
  if (checksum != hatoi(hs)) {
    snprintf(w, 200, "invalid wprot checksum: %s (%ld) != %d (meta=%d, len=%d)",
      	      	    hs, hatoi(hs), checksum, meta, strlen(p)+3);
    debug(3, -1, 226, w);
    return -1; /* exit if invalid checksum */
  }
  hs[2] = ' ';
  
  p   	    = &hs[5];  

  /* check the action type */
  switch (hs[3]) {

    /* version indicator */
    case 'V': get_pword(&p, w);
      	      if (zahl(w)) version = atol(w);
	      else version = 0;
	      if (!valid_wprot_version(version)) {
      	      	debug(2, -1, 226, "invalid wprot version");
	      	return -2;
	      }
      	      return 0;

    /* BBS info */
    case 'B': if (get_wprot_b(&wpb, p, actsender) < 0) return -1;
      	      if (!meta) {
	      	--wpb.hops; /* was increased for the temp file */
		/* don't accept incoming B messages that were created by us */
		if (wpb.hops > 0 && !strcmp(Console_call, wpb.call)) return -1;
      	        /* update hpath.box */
      	      	if (add_wprot_box(wpb.bbs, wpb.timestamp, wpb.status, wpb.hwadr, wpb.sysop)) {
      	      	  /* ok, this info is newer than the last one stored... */
      	      	  add_wprotline(&wpb, meta);
      	      	}
	      } else {
	      	add_wprotline(&wpb, meta);
	      }
      	      return 0;

    /* MYBBS data */
    case 'M': if (get_wprot_m(&wpb, p, actsender) < 0) return -1;
      	      if (!meta) {
	      	--wpb.hops; /* was increased for the temp file */
	      	if (!*wpb.bid || check_double(wpb.bid)) {
      	      	  if (update_mybbsfile(false, wpb.call, &wpb.timestamp, wpb.bbs, "U")) {
              	    if (!*wpb.bid) {
      	      	      new_bid(wpb.bid);
		    }
		    add_hpath(wpb.bbs); /* add hierarchical definition, if known and omitted */
		    add_wprotline(&wpb, meta);
		    create_em_message(&wpb);
	      	  }
      	      	  write_msgid(-1, wpb.bid);
		}
	      } else {
	      	add_wprotline(&wpb, meta);
	      }
      	      return 0;
	      
    /* ERASE data */
    case 'E': if (get_wprot_e(&wpb, p, actsender) < 0) return -1;
	      if (!meta) {
	      	--wpb.hops; /* was increased for the temp file */
	      	if (!*wpb.bid || check_double(wpb.bid)) {
		  /* do not process the own remote erases */
      	      	  if (remote_erase
		      && strcmp(wpb.call, Console_call) ) {
      	      	    if (!remoteerasecheck || strstr(wpb.erasebid, wpb.origin) != NULL)
	      	      add_remote_erase(wpb.origin, wpb.bid, wpb.rxfrom, wpb.erasebid);
      	      	  }
             	  if (!*wpb.bid) {
      	      	    new_bid(wpb.bid);
		  }
		  add_wprotline(&wpb, meta);
		  create_em_message(&wpb);
      	      	  write_msgid(-1, wpb.bid);
		}
	      } else {
	      	add_wprotline(&wpb, meta);
	      }
      	      return 0;

    /* ROUTE data */
    case 'R': if (get_wprot_r(&wpb, p, actsender) < 0) return -1;
	      if (!meta) {
	      	--wpb.hops; /* was increased for the temp file */
		if (!strcmp(Console_call, wpb.call)) {
		  /* don't accept incoming R messages that were created by us */
		  /* but accept those with hops == 0, those are the initial messages! */
		  if (wpb.hops > 0) return -1;
      	      	  if (!do_wprot_routing) return -1;
      	      	  add_wprotline(&wpb, meta);
		} else {
		  qual = get_link_quality_and_status(wpb.rxfrom);
		  if (qual >= WPRINVALID) return 0;
		  wpb.quality += (qual + WPRHOPAGING);
		  wpb.quality += (wpb.quality * WPRPHOPAGING / 100);
		  timestamp = wpb.timestamp;
		  if (wpb.hops < 2) {
		    /* these are our direct neighbours, and we will prefer routing them */
		    /* by using the bare link quality divided by 2 for comparision with */
		    /* other routes   	      	      	      	      	      	      	*/
		    qual = qual / 2;
		  } else {
		    /* for all other routes, we will use the aged quality     	      	*/
		    qual = wpb.quality;
		  }
		  if (add_wprot_routing(wpb.call, wpb.rxfrom, timestamp, qual, wpb.hops)) {
      	      	    if (!do_wprot_routing) return -1;
      	      	    if (wpb.hops == 0) wpb.hops = 1; /* these are our phantom messages */
		    add_wprotline(&wpb, meta);
		  }
		}
	      } else {
	      	add_wprotline(&wpb, meta);
	      }
      	      return 0;

    default : return -1;
  }
}

/* here we create phantom messages for partners without own route broadcast */
/* should be called periodically every some minutes, times operation itself */
void create_own_routing_broadcasts(void)
{
  boolean     created_phantoms;
  time_t      timestamp;
  wprottype   wpb;
  routingtype *rp;
  char	      hs[1000];
  
  if (last_wprot_r > clock_.ixtime) last_wprot_r = clock_.ixtime - WPFREQ_R;
  if (do_wprot_routing && clock_.ixtime - last_wprot_r >= WPFREQ_R) {
    last_wprot_r	= clock_.ixtime;

    if (set_own_wprot_r(hs) >= 0) {  	/* set own routing information */
      process_wprotline(hs, Console_call, false);
    }

    timestamp 	    = ((clock_.ixtime - WPRPHANTOMSUBST) / WPFREQ_R) * WPFREQ_R; /* this is important! */
    /* update routing pointers */
    compare_routing_and_sf_pointers();
    rp	      	    = routing_root;
    created_phantoms= false;
    while (rp != NULL) {
      /* only for neighbours with phantom flag set */
      if (rp->send_phantom) {
      	memset(&wpb, 0, sizeof(wprottype));
      	wpb.version   = WPROT_VERSION;
      	nstrcpy(wpb.call, rp->call, LEN_CALL);
      	nstrcpy(wpb.rxfrom, rp->call, LEN_CALL);
      	wpb.timestamp = timestamp; /* see above, must be full hours UTC */
      	wpb.hops      = 0; /* hops gets increased by 1 in set_wprot_r and */
      	      	      	   /* reduced by 1 in process_wprotline, and 0 is a flag for phantoms! */
      	wpb.quality   = 0; /* gets filled at process_wprotline */
      	wpb.which     = 'R';
      	if (set_wprot_r(hs, &wpb) >= 0) {
	  created_phantoms = true;
      	  /* last parameter must be "false", because we need hops == 0, and that requires the loop */
      	  /* being parsed only one time without intermediate file */
      	  process_wprotline(hs, wpb.rxfrom, false);
      	}
      }
      rp  = rp->next;
    }
    /* if we created phantoms, we have to send as quick as possible a WPROT message to our neighbours! */
    /* because starting from that moment, the routing at our side might have changed, and we have to   */
    /* kick those changes at the other partners with the same phantoms, too   	      	      	       */
    if (created_phantoms) generate_wprot_files();
  }
}

/* some initial values. last_wprot_r gets overwritten in load_bidseek during startup */
void init_wp_timers(void)
{
  last_wprot_r	      	= clock_.ixtime - WPFREQ_R / 2;
}

#define double_val(s, pos) ((s[pos] - '0') * 10 + s[pos + 1] - '0')

/*
On 000201 LU7VBT/U @ LU5VCE.SG.RN.ARG.SA zip 8532 Omar Sierra Grande                               
*/

short process_wpline(char *hs, char *actwpfilesender)
{
  short     d, m, j, h, min, s, k;
  time_t    time;
  char	    zip[256], name[256], qth[256], bbs[256], w[256], call[256];

  if (count_words(hs) < 9)
    return -1;
    
  get_word(hs, w);    /* On    	    */
  get_word(hs, w);    /* timestamp  */
  if (strlen(w) != 6) return -1; /* to prevent from those y2k-buggy systems */
  get_word(hs, call); /* callsign   */
  k     = strpos2(call, "/", 1);
  if (k <= 0 || k != strlen(call) - 1 || call[k] != 'U')
    return -1;

  strdelete(call, k, 2);
  if (!callsign(call))
    return -1;

  j   	= double_val(w, 0);
  m   	= double_val(w, 2);
  d   	= double_val(w, 4);
  h     = 0;
  min   = 1;
  s     = 0;
  if (j >= 0 && j < 38) j += 100;
  time  = calc_ixtime(d, m, j, h, min, s);
  if (!valid_wprot_timestamp(time)) return -1;

  get_word(hs, w);
  if (strcmp(w, "@"))
    return -1;

  get_word(hs, bbs);
  unhpath(bbs, w);
  if (!callsign(w))
    return -1;

  get_word(hs, w); /* "zip" */
  if (strcmp(w, "zip"))
    return -1;

  get_word(hs, zip); /* zip code */
  get_word(hs, name); /* name */
  strcpy(qth, hs);   /* info/qth */
  del_lastblanks(qth);
  if (!*qth) return -1; /* must at least be "?" */
  snprintf(hs, 255, "M @ %s < %s $%s %s %ld %s %s %s",
      	   e_m_verteiler, call, WPDUMMYBID, bbs, time, zip, name, qth);
  sf_rx_emt1(hs, actwpfilesender);
  return 0;
}

#undef double_val
