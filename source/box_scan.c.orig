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
#include "box_scan.h"
#include "boxlocal.h"
#include "filesys.h"
#include "box_logs.h"
#include "box_mem.h"
#include "tools.h"
#include "box_sub.h"
#include "box_sys.h"
#include "boxfserv.h"
#include "sort.h"
#include "box_wp.h"
#include "box_file.h"


static short start_extract(boolean priv, short what, char *s, char *subject,
			   char *sender, char *board, char *name)
{
  short		Result, k, x;
  pathstr	tn;
  char		hs[256], w[256], STR1[256];

  Result	= nohandle;

  if (disk_full) return Result;

  if (what == 0) {  /* autobin */

    strcpy(hs, s);
    x = 1;
    while (x > 0) {
      x = strpos2(hs, "#", 1);
      if (x > 0)
	strdelete(hs, 1, x);
    }
    del_path(hs);
    if (*hs == '\0' || insecure(hs))
      return Result;

    while (strpos2(hs, ".", 1) == 1)
      strdelete(hs, 1, 1);
    if (*hs == '\0')
      return Result;

    lower(hs);
    if (!strcmp(hs, "index"))
      return Result;

    if (priv)
      snprintf(tn, LEN_PATH, "%s%s%s", boxdir, pservdir, hs);
    else {
      if (*expand_fservboard != '\0') {
      	strcpy(w, board);
	check_verteiler(w);
	switch_to_default_fserv_board(w);
	lower(w);
        snprintf(tn, LEN_PATH, "%s%s%s%c%s", boxdir, fservdir, expand_fservboard, dirsep, w);
	if (!exist(tn)) {
	  create_dirpath(tn);
	  if (!exist(tn)) return Result;
	}
        snprintf(tn, LEN_PATH, "%s%s%s%c%s%c%s", boxdir, fservdir, expand_fservboard, dirsep, w, dirsep, hs);
      } else {
        snprintf(tn, LEN_PATH, "%s%snewbin%c%s", boxdir, fservdir, dirsep, hs);
      }
    }
    if (exist(tn))
      return Result;

    k = sfcreate(tn, FC_FILE_RALL);
    if (k < minhandle)
      return Result;

    strcpy(name, tn);
    Result	= k;

  } else {  /* 7plus */

    strcpy(hs, s);
    if (what == 1) {
      for (x = 1; x <= 5; x++)
	get_word(hs, w);
    } else {
      for (x = 1; x <= 2; x++)
	get_word(hs, w);
    }
    del_path(w);
    if (*w == '\0' || insecure(w))
      return Result;

    while (strpos2(w, ".", 1) == 1)
      strdelete(w, 1, 1);
    if (*w == '\0')
      return Result;

    lower(w);
    if (useq(w, "index"))
      return Result;

    if (priv)
      sprintf(tn, "%s%stemp7pl%c%s", boxdir, pservdir, dirsep, w);
    else
      sprintf(tn, "%s%stemp7pl%c%s", boxdir, fservdir, dirsep, w);
    if (exist(tn))
      return Result;

    k	= sfcreate(tn, FC_FILE_RALL);
    if (k < minhandle)
      return Result;

    str2file(&k, s, true);
    strcpy(name, tn);
    Result	= k;
  }

  if (what == 3)
    return Result;

  strcpy(hs, tn);
  x	= strlen(hs);
  while (x > 1 && hs[x - 1] != dirsep)
    x--;
  if (hs[x] == '.')
    return Result;

  strinsert(".", hs, x + 1);
  if (exist(hs))
    return Result;

  k	= sfcreate(hs, FC_FILE_RALL);
  if (k < minhandle)
    return Result;

  sprintf(STR1, "From: %s To: %s", sender, board);
  str2file(&k, STR1, true);
  strcpy(hs, subject);
  cut(hs, 72);
  str2file(&k, hs, true);
  sfclose(&k);

  return Result;
}


static void end_extract(short *k, char *tn, char *ls)
{
  short		x;
  char		hs[256], w[256];

  if (*k < minhandle)
    return;

  sfclose(k);
  strcpy(hs, ls);
  get_word(hs, w);
  get_word(hs, w);
  if (*w == '\0')
    return;
    
  strdelete(w, 1, 1);
  strdelete(w, strlen(w), 1);

  x = strlen(w);
  while (x > 1 && w[x - 1] != '/')
    x--;
  if (x > 1)
    cut(w, x - 1);

  del_path(w);
  if (*w == '\0' || insecure(w))
    return;
 
  while (strpos2(w, ".", 1) == 1)
    strdelete(w, 1, 1);
  if (*w == '\0')
    return;

  lower(w);   /* see above	... */
  if (useq(w, "index"))
    return;

  strcpy(hs, tn);
  get_path(hs);
  strcat(hs, w);
  sfrename(tn, hs);
  strcpy(tn, hs);
}

void check_accepted_bad_rcalls_syntax(char *s)
{
  char	hs[256];
  short x, y;
  
  *accepted_bad_rcalls = '\0';
  nstrcpy(hs, s, 253);
  y = strlen(hs);
  for (x = 0; x < y; x++) if (hs[x] == ',') hs[x] = ' ';
  del_leadblanks(hs);
  del_lastblanks(hs);
  del_mulblanks(hs);
  upper(hs);
  if (*hs) {
    snprintf(accepted_bad_rcalls, 255, " %s ", hs);
  }
}

static boolean accepted_bad_rcall(char *s)
{
  short l;
  char *p;
  
  l = strlen(s);
  if (l < 1) return false;
  if (*s == ' ') return false; /* else sigsegv... */
  if (!*accepted_bad_rcalls) return false;
  p = strstr(accepted_bad_rcalls, s);
  if (p == NULL) return false;
  if (p[-1] != ' ' && p[-1] != '.') return false;
  if (p[l] != ' ' && p[l] != '.') return false;
  return true;
}

void create_my_rline(time_t rxdate, char *bid, char *rline)
{
  char	hs[256], hs3[256];
  
  if (*ownfheader) {
    sprintf(hs3, " [%s", ownfheader);
    if (*bid)
      cut(hs3, MAX_RLINELEN - (38 + strlen(ownhiername)));
    else
      cut(hs3, MAX_RLINELEN - ((38 - 15) + strlen(ownhiername)));
    strcat(hs3, "]");
  } else {
    *hs3 = '\0';
  }
      	      	      	  /* 0123456789012345678 */ 
  ix2string4(rxdate, hs); /* dd:mm:yyyy hh:mm:ss */
  snprintf(rline, MAX_RLINELEN,
      	   "R:%c%c%c%c%c%c/%c%c%c%cz @:%s%s DP%s",
      	   hs[8], hs[9], hs[3], hs[4], hs[0], hs[1],
	   hs[11], hs[12], hs[14], hs[15],
	   ownhiername,
	   hs3,
	   dp_vnr);

  if (*bid) {
    strcat(rline, " $:");
    strcat(rline, bid);
  }
  cut(rline, MAX_RLINELEN);
}

#define double_val(s, pos) ((s[pos] - '0') * 10 + s[pos + 1] - '0')

/* 
991027/2356z
123456789012
012345678901
*/

static boolean check_rdate(char *rdate)
{
  short l;
  
  if (rdate[6] != '/') {
    if (rdate[7] == '/' && rdate[0] == '1' && isdigit(rdate[1]) && isdigit(rdate[2])) { /* fbb 5.15 y2k-bug */
      rdate++;
    } else {
      return false;
    }
  }
  l = strlen(rdate);
  if (l != 11 && (!(l == 12 && isalpha(rdate[11])))) return false;
  if (	 !isdigit(rdate[0]) || !isdigit(rdate[1]) || !isdigit(rdate[2]) || !isdigit(rdate[3])
      || !isdigit(rdate[4]) || !isdigit(rdate[5]) || !isdigit(rdate[7]) || !isdigit(rdate[8])
      || !isdigit(rdate[9]) || !isdigit(rdate[10])) return false;
  l = double_val(rdate, 0); /* year */
  if (l < 70 && l > 38) return false;
  l = double_val(rdate, 2); /* month */
  if (l < 1 || l > 12) return false;
  l = double_val(rdate, 4); /* day */
  if (l < 1 || l > 31) return false;
  l = double_val(rdate, 7); /* hour */
  if (l > 23) return false;
  l = double_val(rdate, 9); /* min */
  if (l > 59) return false;
  return true;
}

/* 
R:991027/2356z
12345678901234
*/

time_t get_headerdate(char *timestr)
{
  int 	      	      	fbby2kbug = 0;
  unsigned short	l, dd, mm, yy, hh, mi, ss;
  boolean     	      	only_date = false;

  l = strlen(timestr);
  if (l < 10 || !check_rdate(&timestr[2])) return 0;
  /* fbb515 y2k bug? */
  if (timestr[2] == '1' && isdigit(timestr[3]) && isdigit(timestr[4])) fbby2kbug = 1;
  if ((l > 13+fbby2kbug) && (timestr[13+fbby2kbug] != ' ') && (upcase_(timestr[13+fbby2kbug]) != 'Z'))
    only_date = true;
  if (!only_date) {
    hh = double_val(timestr, 9+fbby2kbug);
    mi = double_val(timestr, 11+fbby2kbug);
  } else {
    hh = 0;
    mi = 0;    
  }
  ss = 0;
  dd = double_val(timestr, 6+fbby2kbug);
  mm = double_val(timestr, 4+fbby2kbug);
  yy = double_val(timestr, 2+fbby2kbug);
  if (yy < 80)
    yy += 100;
  return (calc_ixtime(dd, mm, yy, hh, mi, ss));
}

#undef double_val

#define MAXFROMLINE 80
#define MAXTOLINE 80

char *get_rtoken(char *rline, char *token, char *para, short maxlen)
{
  char	*p;
  
  *para = '\0';
  p   	= strstr(rline, token);
  if (p != NULL) {
    if (p == rline || p[-1] == ' ' || p[-1] == '\t') {
      p = p + strlen(token);
      nstrcpy(para, p, maxlen);
      del_comment(para, ' ');
      del_comment(para, '\t');
      if (*para) return p;
    }
  }
  return NULL;
}

char *get_w0rli_call(char *rline, char *para)
{
  char	*p;
  
  *para = '\0';
  p   	= strchr(rline, '@');
  if (p != NULL) {
    if (p > rline && isdigit(p[-1])) {
      p++;
      nstrcpy(para, p, LEN_MBX);
      del_comment(para, ' ');
      del_comment(para, '\t');
      if (*para) return p;
    }
  }
  return NULL;
}

void get_rcall(char *rline, char *rcall)
{
  if (get_rtoken(rline, "@:", rcall, LEN_MBX) == NULL) get_w0rli_call(rline, rcall);
}

static boolean analyze_address_headline(char *line, char *token, char *para, short maxlen)
{
  char	*p;
  char	w[256];
  
  p   	= line;
  *para = '\0';
  if (*p && *p != ' ') {
    get_pword(&p, w);
    if (uspos(token, w) == 1) {
      if (strchr(p, ':') != NULL) get_pword(&p, w);
      nstrcpy(para, p, maxlen);
      if (strchr(para, '@') == NULL) *para = '\0';
      else {
      	del_allblanks(para);
      	del_comment(para, '(');
      }
    }
  }
  return (*para != '\0');
}

static boolean strip_invalid_chars(char *s)
{
  char	*p;
  
  p = s;
  while (*s) if (isalnum(*s)) *p++ = *s++; else s++;
  *p = '\0';
  return p == s;
}

static boolean call_in_hpath(char *s)
{
  short   count;
  char	  *p;
  mbxtype rcall;
  
  count   = 0;
  p   	  = rcall;
  while (*s && count++ < LEN_MBX) {
    if (*s == '.') {
      *p    = '\0';
      s++;
      if (count > 0 && callsign(rcall)) return true;
      p     = rcall;
      count = 0;
    } else {
      *p++  = *s++;
    }
  }
  *p  	  = '\0';
  if (count > 0 && callsign(rcall)) return true;
  return false;
}

static boolean accepted_distchange(char *olddist, char *newdist)
{
  if (	 !strcmp(olddist, "WWW")
      || !strcmp(olddist, "ALLBBS")
      || !strcmp(olddist, "ALL")
      || !strcmp(olddist, "AMSAT")
      || !strcmp(olddist, "ARRL")
      || !strcmp(olddist, "ARL")  ) {
    if (   !strcmp(newdist, "WW")
      	|| !strcmp(newdist, "ARRL") ) {
      return true;
    }
  }
  return false;
}

static boolean check_sanity(char *puffer, long size, char *absender, char *board,
      	      	      	    char *mbx, char *bid, char *subject, char msgtype,
			    char *reason)
{
  long	    rp, ct, ctb;
  short     bidchanges;
  boolean   blankline, rline, atcall, fromok, took, bidchange;
  char	    *p;
  calltype  fromcall, call, lastbidokat;
  boardtype toboard;
  mbxtype   frombbs, sendmbx, rcall;
  bidtype   rbid, lastbid;
  char	    rdate[15];
  char	    fromline[MAXFROMLINE+1], toline[MAXTOLINE+1];
  char	    hs[256], w[256], w2[256];

  debug0(1, 0, 218);

  rp  	    = 0;
  blankline = false;
  rline     = true;
  fromok    = false;
  took	    = false;
  bidchange = false;
  bidchanges= 0;
  ct  	    = 0;
  ctb 	    = 0;
  *fromline = '\0';
  *toline   = '\0';
  *fromcall = '\0';
  *toboard  = '\0';
  *sendmbx = '\0';
  *frombbs  = '\0';
  strcpy(lastbid, bid);
  *lastbidokat= '\0';
  unhpath(mbx, hs);
  atcall    = callsign(hs);
  
  while (rp < size && ct < 500 && ctb < 10) { /* check for max. 500 lines of content, if header is corrupted */

    get_line(puffer, &rp, size, hs);

    if (!*hs) blankline = true; /* OK, header parsed */

    if (!blankline) { /* still in header lines */

      if (hs[0] == 'R' && get_rtoken(hs, "R:", rdate, 15) != NULL) {

      	*call 	= '\0';
      	*rcall	= '\0';
      	*rbid 	= '\0';
	
      	if (stop_broken_rlines && !atcall && !rline) { /* this is an R: line, but previous line was not, although we still are in headers */
	  strcpy(reason, "broken R: lines");
	  return false;
	}
	
	if (stop_invalid_rdates  && !atcall && !check_rdate(rdate)) {
	  snprintf(reason, 80, "corrupted date in R: line : %s", rdate);
	  return false;
	}
	
      	get_rcall(hs, rcall);
      	if (*rcall) unhpath(rcall, call);
	
      	if (stop_invalid_rcalls && !atcall && *rcall) {
	  if (!callsign(call) && !accepted_bad_rcall(call) && !accepted_bad_rcall(rcall)
	      && !call_in_hpath(rcall)) {
	    snprintf(reason, 80, "invalid callsign in forward path: %s", rcall);
	    return false;
	  }
	}
	
	/* check for changed BIDs (but wait until last r:line for alarm) */
	if (stop_changed_bids && !atcall && *lastbid && get_rtoken(hs, "$:", rbid, LEN_BID) != NULL) {
          bidchange = !useq(lastbid, rbid);
	  if (bidchange) {
	    bidchanges++;
	    strcpy(lastbid, rbid);
	  }
	}

      } else rline = false;
    }

    if (blankline || !rline) ct++;
    if (blankline) ctb++;

    /* following all those content related checks */

    if (!*fromline)
      analyze_address_headline(hs, "From", fromline, MAXFROMLINE);

    if (!*toline)
      analyze_address_headline(hs, "To", toline, MAXTOLINE);
    
    if (!atcall) {

      if (stop_changed_senders && !fromok && *fromline) {
	fromok = true;
	strcpy(w, fromline);
	del_comment(w, '@');
	del_lastblanks(w);
	if (!useq(w, absender) && callsign(w)) {
	  snprintf(reason, 80, "sender was changed from %s to %s", w, absender);
	  return false;
	}
      }
      
      if (!took && *toline) {
	took = true;
	if (stop_changed_boards) {
	  strcpyupper(w, toline);
	  del_comment(w, '@');
	  del_comment(w, '%'); /* for those sendmail converters */
	  if (strncmp(w, board, 6) && !callsign(w)) {
	    if (strip_invalid_boardname_chars(w) && strncmp(w, board, 6) && !callsign(w)) {
	      if (strstr(board, w) != board) {
		snprintf(reason, 80, "board was changed from %s to %s", w, board);
		return false;
	      }
	    }
	  }
	}
	if (stop_changed_mbx) {
	  p   = strchr(toline, '@');
	  if (p != NULL) {
	    strcpyupper(w, &p[1]);
      	    del_comment(w, '.');
	    if (strip_invalid_chars(w) && *w) {
	      strcpyupper(w2, mbx);
	      del_comment(w2, '.');
	      strip_invalid_chars(w2);
	      if (strcmp(w, w2)) {
	      	if (!callsign(w) && !callsign(w2)) {
		  if (!accepted_distchange(w, w2)) {
		    snprintf(reason, 80, "distribution was changed from @%s to @%s", w, mbx);
		    return false;
		  }
		}
	      }
	    }
	  }
	}
      }
      
    }

  } /* end of while loop */

  if (stop_changed_bids && bidchanges) {
    if (!useq(lastbid, bid)) { /* maybe someone recovered the original BID */
      if (bidchanges > 1) {
        snprintf(reason, 80, "BID was changed %d times, from initially %s to finally %s", bidchanges, lastbid, bid);
      } else {
      	snprintf(reason, 80, "BID was changed from %s to %s", lastbid, bid);
      }
/*      if (*lastbidokat) {
      	if (bidchanges > 1) strcat(reason, " (first change probably was at bbs ");
      	else strcat(reason, " (probably at bbs ");
      	strcat(reason, lastbidokat);
      	strcat(reason, ")");
      }
*/      return false;
    }
  }

  return true;
}

#undef MAXFROMLINE
#undef MAXTOLINE

/* Diese Prozedur liest eine bereits im Speicher befindliche Nachricht   */
/* und versucht aus dem Nachrichtenkopf die Absender-BBS zu ersehen.     */
/* Ausserdem wird ueberprueft, ob es sich eventuell um ein DieBox -      */
/* Binaerfile handelt. Falls ja, so wird der Offset zum Dateianfang des  */
/* Starts der binaeren Daten zurueckgegeben.                             */
/* Ab v4.06 ebenfalls Scan nach WP-Infos.                                */
/* Ab v4.07 ebenfalls Scan nach //E                                      */
/* Ab v4.18 7plus/AutoBIN - Extraktor                                    */
/* Ab v5.03 und guess_mybbs == true werden auch mybbs-infos extrahiert   */
/* Ab v5.08.11 erweiterter Test auf korrekte R:-Zeilen	      	      	 */
/* Ab v5.08.14 Scan nach WPROT-Infos  	      	      	      	      	 */

boolean scan_for_ack(char *puffer, long size, boolean wpupdate,
      	      	     boolean wprotupdate, boolean part,
                     char *absender, char *board, char *subject, char *mbx,
		     char *bid, char msgtype, char *ackcall, boolean *is_binary,
		     boolean *is_dirty, boolean *is_html, char *dirtystring,
		     boolean *is_7plus, boolean *is_broken)
{
  short		x, lct, lastrl, a, extracthandle;
  boolean	rok, priv, is_ack;
  long		binstart; /* now a dummy value */
  long		rp, lp, lp2, li;
  time_t	txdate;
  pathstr	extractname;
  char		hs[1000], w[256], shs[256], hsu[256];
  calltype	actwpfilesender;

  binstart	= 0;
  *is_binary	= false;
  *is_7plus	= false;
  is_ack	= false;
  *is_html	= false;
  *is_broken  	= false;
  *ackcall	= '\0';
  *shs		= '\0';
  *extractname	= '\0';
  rp		= 0;
  lp2		= 0;
  lct		= 0;
  txdate	= 0;
  lastrl	= 0;
  rok		= false;
  priv		= false;
  a		= -1;
  extracthandle	= nohandle;

  if (wpupdate || wprotupdate) {
    if (strlen(absender) <= LEN_CALL)
      strcpy(actwpfilesender, absender);
    else {
      wpupdate = false;
      wprotupdate = false;
    }
  }

  if (check_rheaders) {
    *is_broken  	= !check_sanity(puffer, size, absender, board, mbx, bid,
      	      	      	      	      	subject, msgtype, dirtystring);
  }
  
  debug0(2, 0, 66);

  while (rp < size && lct == lastrl) {
    lct++;
    lp		= rp;
    get_line(puffer, &rp, size, hs);
    if (hs[0] == 'R' && hs[1] == ':') {
      if (strpos2(hs, "@", 1) > 10) {
	lastrl	= lct;
	lp2	= lp;
	rok	= true;
      }
    }
  }

  if (rok) get_line(puffer, &lp2, size, shs);

  if (*shs != '\0') {
    get_word(shs, w); /* Datum */
    if (guess_mybbs) {
      txdate	= get_headerdate(w);
      if (txdate > clock_.ixtime) {
      	if (txdate < clock_.ixtime - 180*60) {
          txdate  = clock_.ixtime;
	} else {
	  txdate  = 0;
	}
      }
    }
      
    get_word(shs, w);
    x		= strpos2(w, "@", 1);
    if (x > 0) {
      strdelete(w, 1, x);
      if (w[0] == ':')
	strdelete(w, 1, 1);
      upper(w);
      unhpath(w, hs);
      if (!callsign(hs))
	*ackcall= '\0';
      else
	strcpy(ackcall, hs);
    }
  }

  x	= 0;
  while (rp < size && !*is_binary && !*is_dirty && !*is_broken) {
    x++;
    get_lline(puffer, &rp, size, hs, 999);

    if (extracthandle >= minhandle)
      str2file(&extracthandle, hs, true);

    if (*hs == '\0')  /* empty line nicht auswerten */
      continue;

    if (hs[0] == '#' && hs[1] == 'B' && true_bin(hs) >= 0) {
      *is_binary	= true;
      binstart		= rp;
      if (!autobinextract || part)
	continue;

      priv		= (strcmp(Console_call, board) == 0);
      if (!(priv || msgtype == 'B'))
	continue;

      cut(hs, 255);
      extracthandle	= start_extract(priv, 0, hs, subject, absender, board, extractname);
      if (extracthandle >= minhandle) {
	li		= size - rp;
	if (sfwrite(extracthandle, li, (char *)(&puffer[rp])) != li)
	  sfclosedel(&extracthandle);
	else
	  sfclose(&extracthandle);
      }
      continue;
    }
    
    if (a >= 0) {

      if (hs[0] != ' ' || hs[1] != 's')
      	continue;
	
      if ((a != 0 || strstr(hs, " stop_7+. ") != hs) &&
	  (a != 1 || strstr(hs, " stop_text.") != hs) &&
	  (a != 2 || strstr(hs, " stop_info.") != hs))
	continue;

      if (extracthandle >= minhandle) {
      	cut(hs, 255);
	strcpy(w, extractname);
	end_extract(&extracthandle, extractname, hs);
	sig_7plus_incoming(priv, extractname, w, board);
      }
      a			= -1;
      continue;

    }

    if (hs[0] == ' ' && hs[1] == 'g' && hs[2] == 'o' && hs[3] == '_') {
    
      if (strstr(hs, " go_7+. ") == hs) {
	*is_7plus		= true;
	binstart		= rp;
	a			= 0;
	cut(hs, 255);
	if (auto7plusextract && !part) {
	  priv		= (strcmp(Console_call, board) == 0);
	  if (priv || msgtype == 'B')
	    extracthandle	= start_extract(priv, 1, hs, subject, absender, board, extractname);
	}
	continue;
      }

      if (strstr(hs, " go_text. ") == hs) {
      	cut(hs, 255);
	if (uspos(".ERR", hs) > 0) {
	  *is_7plus	= true;
	  binstart	= rp;
	  a		= 1;
	  continue;
	}

	if (uspos(".COR", hs) <= 0)
	  continue;

	*is_7plus		= true;
	binstart		= rp;
	a 		= 1;
	if (auto7plusextract && !part) {
	  priv 		= (strcmp(Console_call, board) == 0);
	  if (priv || msgtype == 'B')
	    extracthandle = start_extract(priv, 2, hs, subject, absender, board, extractname);
	}
	continue;
      }

      if (strstr(hs, " go_info. ") == hs) {
	a			= 2;
	cut(hs, 255);
	if (auto7plusextract && !part) {
	  priv		= (strcmp(Console_call, board) == 0);
	  if (priv || msgtype == 'B')
	    extracthandle	= start_extract(priv, 3, hs, subject, absender, board, extractname);
	}
	continue;
      }
    
    }

    if (hs[0] == '/') {

      if (strstr(hs, "/ACK") == hs) {
      	del_lastblanks(hs);
        if (!strcmp(hs, "/ACK"))
	  is_ack		= true;
        continue;
      }

      if (strstr(hs, "/ack") == hs) {
	del_lastblanks(hs);
	if (!strcmp(hs, "/ack"))
	  is_ack		= true;
	continue;
      }

      if (hs[1] == '/' && (hs[2] == 'E' || hs[2] == 'e')) {
	cut(hs, 255);
	strcpy(hsu, hs);
	get_word(hsu, w);
	upper(w);
	strcpy(hsu, "//ECHO");
	if (strstr(hsu, w) == hsu) {
	  *is_dirty	= true;
	  strcpy(dirtystring, hs);
	  continue;
	}
      }

    }

    if (hs[0] == '<' && (hs[1] == 'H' || hs[1] == 'h')) {
      if (strstr(hs, "<HTML>") != NULL || strstr(hs, "<html>") != NULL) {
        *is_html		= true;
        continue;
      }
    }

    if (wprotupdate && isxdigit(hs[0]) && isxdigit(hs[1]) && hs[2] == ' ') {
      if (process_wprotline(hs, actwpfilesender, true) == -2) wprotupdate = false;
      continue;
    }
    
    if (wpupdate && hs[0] == 'O' && hs[1] == 'n' && hs[2] == ' ') {  /* WP */
      cut(hs, 255);
      process_wpline(hs, actwpfilesender);
      continue;
    }

    if (badwordroot == NULL)
      continue;

    if (private_dirtycheck || msgtype != 'P') {
      cut(hs, 255);
      strcpyupper(hsu, hs);
      if (check_for_dirty(hsu)) {
        *is_dirty	= true;
        strcpy(dirtystring, hs);
      }
    }

  }

  if (extracthandle >= minhandle)
    sfclosedel(&extracthandle);

  if (wpupdate || wprotupdate)
    *actwpfilesender	= '\0';
    
  if (guess_mybbs && txdate > 0 && callsign(absender) && callsign(ackcall))
    update_mybbsfile(false, absender, &txdate, ackcall, "G");
  
  return is_ack;
}


